//////////////////////////////////////////////////////////////////////////////////////
// fmesh.cpp - Fang mesh module.
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
// 01/24/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fmesh.h"
#include "fsh.h"
#include "frenderer.h"
#include "floop.h"
#include "fclib.h"
#include "fmesh_coll.h"
#include "fresload.h"
#include "fliquid.h"
#include "fshadow.h"

#include "fshaders.h"
#include "fvis.h"

#include "fperf.h"

#if !FANG_PRODUCTION_BUILD
#include "fdraw.h"
#endif

#if FANG_PLATFORM_DX
	#include "dx/fdx8mesh.h"
#elif FANG_PLATFORM_GC
	#include "gc/fGCmesh.h"
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	#include "ps2/fps2mesh.h"
//ARG - <<<<<
#else
	#error <Fang Error: No mesh header available for platform.>
#endif


//////////////////////////////////////////////////////////////////////////////////////
// Local Defines:
//////////////////////////////////////////////////////////////////////////////////////

#define _KDOP_GEO_POOL_SIZE						64
#define _KDOP_FACE_POOL_SIZE 					( _KDOP_GEO_POOL_SIZE * 14 )

#define _MAX_TC_VALUE_TO_ADJUST_BY_LOOPING		5.0f

CFMeshInst *FMesh_PlayerMesh=NULL;

//////////////////////////////////////////////////////////////////////////////////////
// Local Structures:
//////////////////////////////////////////////////////////////////////////////////////

struct _RendererState_t
{
	CFColorMotif AmbientMotif;
	CFColorRGB AmbientRGB;
	f32 fAmbientIntensity;
	
};

//extern BOOL FVis_bInRenderTarget;

void fmesh_Renderer_SetDefaultState( void );

//////////////////////////////////////////////////////////////////////////////////////
// Global Variables
//////////////////////////////////////////////////////////////////////////////////////

BOOL FMesh_bForceMinLOD;
s32  FMesh_nForceLOD;

// Maximum number of mesh lights allowed by the hardware (will be overridden by platform specific code)
u32 FMesh_nMaxMeshLights = 6;

// These variables are private to the fmesh system, but need to be shared with the
// platform-specific components of the fmesh system:
u32 				FMesh_nLightCount;
u32 				FMesh_nMaxLightCount;
FMesh_RenderLight_t *FMesh_paLightList;

CFColorMotif 		FMesh_AmbientMotif;
CFColorRGB 			FMesh_AmbientRGB;
f32 				FMesh_fAmbientIntensity;

BOOL FMesh_bRenderShadows=FALSE;
CFLight *FMesh_CurrentLight=NULL;
CFVec3 FMesh_vShadowCam(0,0,0);

#if !FANG_PRODUCTION_BUILD
	u32 FMesh_nDrawCollisionGeoFlags;
#endif


//////////////////////////////////////////////////////////////////////////////////////
// Static Variables
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _bModuleInitialized;

static CFXfm _XfmPosterY;
static CFXfm _XfmPosterX;
static CFXfm _XfmPosterZ;

static CFMtx43 _TextureMtx[FSH_MAX_TEX_LAYER_COUNT];
static CFColorMotif _AngularEmissive;

static CFLight **_apLightListSort;
static f32 *_afLightMag2;

static FkDOP_Face_t *_akDOPFacePool;
static FkDOP_Geo_t *_akDOPGeoPool;
static u32 _nkDOPFacePoolIdx;
static u32 _nkDOPGeoPoolIdx;

static u32 _nTotalLights=0;

#if !FANG_PRODUCTION_BUILD
	static u32 _nBufferWarningFrame = 0;
#endif


//////////////////////////////////////////////////////////////////////////////////////
// Static Functions
//////////////////////////////////////////////////////////////////////////////////////

//static void _FindMostInfluentialLights( const CFVec3 *pSortLightsToThisPoint_WS, CFLight **ppLightList, u32 nLightCount, u32 nAvailableSlots );



//////////////////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////////////////


//
//
//
BOOL fmesh_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

#if !FANG_PRODUCTION_BUILD
	FMesh_nDrawCollisionGeoFlags = FMESH_DRAW_COLL_GEO_DISABLED;
//	FMesh_nDrawCollisionGeoFlags = FMESH_DRAW_COLL_GEO_LEAF_TRIS;
#endif

	FMesh_nForceLOD = -1;
	FMesh_bForceMinLOD = FALSE;

	FMesh_nLightCount = 0;
	if ( Fang_ConfigDefs.nWorld_MaxLightsPerDraw < 1 ) 
	{
		DEVPRINTF( "fmesh_ModuleStartup(): Fang_ConfigDefs.nWorld_MaxLightsPerDraw must be a minimum of 1. Value clamped.\n" );
		FMesh_nMaxLightCount = 1;
	} 
	else if ( Fang_ConfigDefs.nWorld_MaxLightsPerDraw > 65535 ) 
	{
		DEVPRINTF( "fmesh_ModuleStartup(): Fang_ConfigDefs.nWorld_MaxLightsPerDraw must be a maximum of 65535. Value clamped.\n" );
		FMesh_nMaxLightCount = 65535;
	} 
	else 
	{
		FMesh_nMaxLightCount = Fang_ConfigDefs.nWorld_MaxLightsPerDraw;
	}

	FMesh_paLightList = (FMesh_RenderLight_t *)fres_Alloc( FMesh_nMaxLightCount * sizeof(FMesh_RenderLight_t) );
	if ( FMesh_paLightList == NULL ) 
	{
		// Out of memory...
		DEVPRINTF( "fmesh_ModuleStartup(): Could not allocate %u bytes.\n", FMesh_nMaxLightCount * sizeof(FMesh_RenderLight_t) );
		return FALSE;
	}

	_akDOPGeoPool = (FkDOP_Geo_t *)fres_AllocAndZero( sizeof( FkDOP_Geo_t ) * _KDOP_GEO_POOL_SIZE );
	if ( !_akDOPGeoPool )
	{
		return FALSE;
	}
	_nkDOPGeoPoolIdx = 0;
	
	_akDOPFacePool = (FkDOP_Face_t *)fres_AllocAndZero( sizeof( FkDOP_Face_t ) * _KDOP_FACE_POOL_SIZE );
	if ( !_akDOPFacePool )
	{
		return FALSE;
	}
	_nkDOPFacePoolIdx = 0;
	
	_XfmPosterX.Identity();
	_XfmPosterY.Identity();
	_XfmPosterZ.Identity();

	_bModuleInitialized = TRUE;

	// Clear the texture matrices
	u32 i;
	for ( i = 0; i < FSH_MAX_TEX_LAYER_COUNT; i++ )
	{
		_TextureMtx[i].Identity();
	}

	_AngularEmissive.nMotifIndex = 0;

	fmesh_Renderer_SetDefaultState();

	fmesh_ResetLightList();
	
	return TRUE;
}


//
//
//
void fmesh_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	_bModuleInitialized = FALSE;
}


//
//
//
void fmesh_Ambient_Set( float fRed, float fGreen, float fBlue, float fIntensity ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT_UNIT_FLOAT( fRed );
	FASSERT_UNIT_FLOAT( fGreen );
	FASSERT_UNIT_FLOAT( fBlue );
	FASSERT_UNIT_FLOAT( fIntensity );

	FMesh_AmbientRGB.Set( fRed, fGreen, fBlue );
	FMesh_fAmbientIntensity = fIntensity;

	FMesh_AmbientMotif.ColorRGB = FMesh_AmbientRGB * FMesh_fAmbientIntensity;
}


//
//
//
void fmesh_Ambient_Set( const CFColorRGB *pColorRGB, float fIntensity ) 
{
	FASSERT( _bModuleInitialized );
	FCOLOR_FASSERT_UNIT_RGB( pColorRGB );
	FASSERT_UNIT_FLOAT( fIntensity );

	FMesh_AmbientRGB = *pColorRGB;
	FMesh_fAmbientIntensity = fIntensity;

	FMesh_AmbientMotif.ColorRGB = FMesh_AmbientRGB * FMesh_fAmbientIntensity;
}


//
//
//
void fmesh_Ambient_Get( float *pfRed, float *pfGreen, float *pfBlue, float *pfIntensity ) 
{
	FASSERT( _bModuleInitialized );

	if ( pfRed ) 
	{
		*pfRed = FMesh_AmbientRGB.fRed;
	}
	if ( pfGreen ) 
	{
		*pfGreen = FMesh_AmbientRGB.fGreen;
	}
	if ( pfBlue ) 
	{
		*pfBlue = FMesh_AmbientRGB.fBlue;
	}
	if ( pfIntensity ) 
	{
		*pfIntensity = FMesh_fAmbientIntensity;
	}
}


//
//
//
void fmesh_Ambient_Get( float *pfRed, float *pfGreen, float *pfBlue ) 
{
	FASSERT( _bModuleInitialized );

	if ( pfRed ) 
	{
		*pfRed = FMesh_AmbientMotif.ColorRGB.fRed;
	}
	if ( pfGreen ) 
	{
		*pfGreen = FMesh_AmbientMotif.ColorRGB.fGreen;
	}
	if ( pfBlue ) 
	{
		*pfBlue = FMesh_AmbientMotif.ColorRGB.fBlue;
	}
}


//
//
//
void fmesh_Ambient_Get( CFColorRGB *pColorRGB, float *pfIntensity ) 
{
	FASSERT( _bModuleInitialized );

	if ( pColorRGB ) 
	{
		*pColorRGB = FMesh_AmbientRGB;
	}
	if ( pfIntensity ) 
	{
		*pfIntensity = FMesh_fAmbientIntensity;
	}
}


//
//
//
void fmesh_Ambient_Get( CFColorRGB *pColorRGB ) 
{
	FASSERT( _bModuleInitialized );

	if ( pColorRGB ) 
	{
		*pColorRGB = FMesh_AmbientMotif.ColorRGB;
	}
}


//
//
//
void fmesh_ResetLightList( void ) 
{
	FASSERT( _bModuleInitialized );

	FMesh_nLightCount = 0;

	_nTotalLights = 0;
}


//
//
//
u32 fmesh_GetLightCount( void ) 
{
	FASSERT( _bModuleInitialized );

	return FMesh_nLightCount;
}


//
//
//
u32 fmesh_Renderer_GetStateSize( void ) 
{
	FASSERT( _bModuleInitialized );
	return sizeof(_RendererState_t);
}


//
//
//
void fmesh_Renderer_GetState( void *pDestState ) 
{
	_RendererState_t *pMeshState = (_RendererState_t *)pDestState;

	FASSERT( _bModuleInitialized );

	pMeshState->AmbientMotif = FMesh_AmbientMotif;
	pMeshState->AmbientRGB = FMesh_AmbientRGB;
	pMeshState->fAmbientIntensity = FMesh_fAmbientIntensity;
}


//
//
//
void fmesh_Renderer_SetState( const void *pState ) 
{
	const _RendererState_t *pMeshState = (const _RendererState_t *)pState;

	FASSERT( _bModuleInitialized );

	FMesh_AmbientMotif = pMeshState->AmbientMotif;
	FMesh_AmbientRGB = pMeshState->AmbientRGB;
	FMesh_fAmbientIntensity = pMeshState->fAmbientIntensity;
}


//
//
//
void fmesh_Renderer_SetDefaultState( void ) 
{
	FASSERT( _bModuleInitialized );

	FMesh_AmbientMotif = 0;
	FMesh_AmbientRGB.Black();
	FMesh_fAmbientIntensity = 0.0f;
}


// Searches the mesh for the specified bone name. Returns the index
// if found, or -1 if not found.
//
s32 fmesh_FindBone( const FMesh_t *pMesh, cchar *pszBoneName )
{
	if ( pszBoneName == NULL )
	{
		return -1;
	}

	if ( pMesh == NULL )
	{
		return -1;
	}

	s32 i;

	for ( i = 0; i < pMesh->nBoneCount; ++i )
	{
		if( !fclib_stricmp( pszBoneName, pMesh->pBoneArray[i].szName ) )
		{
			return i;
		}
	}

	return -1;
}

//-------------------------------------------------------------------------------------------------------------------
// CFMeshInst:
//-------------------------------------------------------------------------------------------------------------------

//
//
//
CFMeshInst::CFMeshInst( void ) 
{ 
	m_pMesh = NULL; 
	m_apBoneMtxPalette = NULL;
	m_nkDOPBufferStartIdx = 0xffff;
	m_pRenderLights = NULL;
	m_nRenderLightCount = 0;
	m_nLowestWeightedLightIdx = -1;
	m_nCullDirection = FMESH_CULLDIR_CW;
	
	#if FSH_DYNAMIC_SREFLECT
		m_bReflect = FALSE;
		m_pReflectTex = NULL;
	#endif
	
	m_aTexLayerID = NULL; 
	m_aAnimTC = NULL; 
	m_aTexFlip = NULL; 
	#if FANG_WINGC
	m_pDrawFilter = NULL;
	m_pDrawFilterData = NULL;
	#endif
	m_pPreDrawCallback = NULL; 
	m_pPostDrawCallback = NULL; 
	m_pPreDrawData = NULL; 
	m_pPostDrawData = NULL;
	m_nCreatedPartsMask = m_nDrawnPartsMask = m_nCollPartsMask = 0xffffffff;
	m_nDrawnBonesCount = m_nCreatedBonesCount = 0;
	m_MeshRGBA.Set( 1.f, 1.f, 1.f, 1.f );
	m_fShadowIntensity = 1.f;
	
	m_panLightRegisterLMOverride = NULL;

	m_nPixelRadiusLastRender = 640;

	m_nCurrentLOD = 0;
	m_nColorStreamCount = 0;
	m_papColorStreams = NULL;
	m_pProcedural = NULL;

	#if FANG_PLATFORM_GC		
		m_nStartingMatrixBufferIdx = 0xffff;
		m_nStartingSkinVertexBufferIdx = 0xffff;
	#elif FANG_PLATFORM_DX
		m_PosMtx.Identity();
		m_fMatrixScale = 1.f;
	#endif			
	
	InvalidateFrameLastDrawn(); 
}


// If bNoMemAlloc is TRUE, the init function will not allocate any memory. This
// is useful when a CFMeshInst is to be used to render different meshes. However,
// since no memory is allocated, there can be no bones and no texture layer info.
//
//
void CFMeshInst::Init( FMesh_t *pMesh, BOOL bAnimWhenPaused/*=FALSE*/, u32 nDrawnPartsMask/*=0xffffffff*/, u32 nCollisionPartsMask/*=0xffffffff*/, BOOL bNoMemAlloc/*=FALSE*/ ) 
{
	m_pMesh = pMesh;

	m_nFlags = FMESHINST_FLAG_NOCOLLIDE | FMESHINST_FLAG_NOBONES;
	if ( bAnimWhenPaused ) 
	{
		m_nFlags |= FMESHINST_FLAG_ANIM_WHEN_PAUSED;
	}

	m_fCullDist = FMATH_MAX_FLOAT;
	m_Xfm.Identity();
#if FANG_PLATFORM_DX
	m_PosMtx.Identity();
	m_fMatrixScale = 1.f;
#endif

	if ( pMesh ) 
	{
		m_BoundSphere_MS = pMesh->BoundSphere_MS;
	}
	
	// Determine Collision mask
	m_nCollPartsMask = nCollisionPartsMask & nDrawnPartsMask;

	// Determine the number of drawn bones
	m_nDrawnPartsMask = m_nCreatedPartsMask = nDrawnPartsMask;
	m_nCreatedBonesCount = 0;
	if ( m_pMesh )
	{	
		if ( nDrawnPartsMask == 0xffffffff )
		{
			m_nCreatedBonesCount = m_pMesh->nBoneCount;
		}
		else
		{
			u32 i;
			for ( i = 0; i < m_pMesh->nUsedBoneCount; i++ )
			{
				// If this bone is drawn in this instance, increment the counter
				if ( (1 << m_pMesh->pBoneArray[i].nPartID) & nDrawnPartsMask )
				{
					m_nCreatedBonesCount++;
				}
			}
		}
	}
	m_nDrawnBonesCount = m_nCreatedBonesCount;
	
	m_nFrameOfLastAnimateLayer = 0;
	m_nLastAnimateLayerTicks = 0;
	AllocInstArrays( bNoMemAlloc );
}


//
//
//
void CFMeshInst::Init( FMeshInit_t *pMeshInit, u32 nDrawnPartsMask/*=0xffffffff*/, u32 nCollisionPartsMask/*=0xffffffff*/, BOOL bNoMemAlloc/*=FALSE*/ ) 
{
	m_pMesh = pMeshInit->pMesh;
	m_nFlags = pMeshInit->nFlags | FMESHINST_FLAG_NOBONES;
	m_fCullDist = pMeshInit->fCullDist;
	m_Xfm.BuildFromMtx( pMeshInit->Mtx, TRUE );
	
#if FANG_PLATFORM_DX
	m_PosMtx.Set( pMeshInit->Mtx );
	m_fMatrixScale = 1.f;
#endif			

	if ( pMeshInit->pMesh ) 
	{
		m_BoundSphere_MS = pMeshInit->pMesh->BoundSphere_MS;
	}

	// Determine Collision mask
	m_nCollPartsMask = nCollisionPartsMask & nDrawnPartsMask;

	// Determine the number of active bones
	m_nDrawnPartsMask = m_nCreatedPartsMask = nDrawnPartsMask;
	m_nCreatedBonesCount = 0;
	if ( m_pMesh )
	{	
		if ( nDrawnPartsMask == 0xffffffff )
		{
			m_nCreatedBonesCount = m_pMesh->nBoneCount;
		}
		else
		{
			u32 i;
			for ( i = 0; i < m_pMesh->nUsedBoneCount; i++ )
			{
				// If this bone is active in this instance, increment the counter
				if ( (1 << m_pMesh->pBoneArray[i].nPartID) & nDrawnPartsMask )
				{
					m_nCreatedBonesCount++;
				}
			}
		}
	}
	m_nDrawnBonesCount = m_nCreatedBonesCount;
	
	m_nFrameOfLastAnimateLayer = 0;
	m_nLastAnimateLayerTicks = 0;
	AllocInstArrays( bNoMemAlloc );
}


//
//
//
BOOL CFMeshInst::AllocInstArrays( BOOL bNoMemAlloc ) 
{
	FResFrame_t ResFrame;
	u32 i, nByteCount;

	ResFrame = fres_GetFrame();

	if ( m_pMesh ) 
	{
		m_bReflect = FALSE;

		m_apBoneMtxPalette = NULL;
		m_aTexLayerID = NULL;
		m_aAnimTC = NULL;
		m_aTexFlip = NULL;

		// Init bone palette...
		if ( m_pMesh->nBoneCount && !bNoMemAlloc ) 
		{
			// This mesh has a bone palette...

			nByteCount = m_pMesh->nBoneCount * sizeof(CFMtx43A *);
			nByteCount = FMATH_BYTE_ALIGN_UP( nByteCount, CFMtx43A::CLASS_BYTE_ALIGNMENT );

			m_apBoneMtxPalette = (CFMtx43A **)fres_AlignedAlloc( nByteCount + (m_nCreatedBonesCount * sizeof(CFMtx43A)), CFMtx43A::CLASS_BYTE_ALIGNMENT );
			if ( m_apBoneMtxPalette == NULL ) 
			{
				goto _ExitInitMeshWithError;
			}

			u32 nMatrixIdx = 0;
			for ( i=0; i<m_pMesh->nBoneCount; i++ ) 
			{
				if ( m_nCreatedPartsMask & (1 << m_pMesh->pBoneArray[i].nPartID) )
				{
					// We only assign a matrix to the bones that will be used
					m_apBoneMtxPalette[i] = ((CFMtx43A *)( (u32)m_apBoneMtxPalette + nByteCount )) + nMatrixIdx;
					m_apBoneMtxPalette[i]->Identity();
					nMatrixIdx++;
				}
				else
				{
					m_apBoneMtxPalette[i] = NULL;
				}
			}
			
			// Make sure that we assigned the same number of matrices as were created
			FASSERT( nMatrixIdx == m_nCreatedBonesCount );
		}

		m_nFrameOfLastAnimateLayer = FVid_nFrameCounter - 1;

		FMATH_SETBITMASK( m_nFlags, FMESHINST_FLAG_SAMPLE_ANIMLAYER_TICKS );

		if ( m_pMesh->nTexLayerIDCount && !bNoMemAlloc ) 
		{
			// Init texture layer ID table...
			m_aTexLayerID = (FMeshInstTexLayerID_t *)fres_AllocAndZero( m_pMesh->nTexLayerIDCount * sizeof(FMeshInstTexLayerID_t) );
			if ( m_aTexLayerID == NULL ) 
			{
				goto _ExitInitMeshWithError;
			}

			// Set the default values for the texlayerids			
			for ( i = 0; i < m_pMesh->nTexLayerIDCount; i++ )
			{
				m_aTexLayerID[i].nLayerAlpha = 255;
			}

			// Init texture animation palette...
			if ( m_pMesh->nTexLayerIDCount_ST ) 
			{
				// This mesh has a texture coordinate animation palette...

				m_aAnimTC = fnew CFMeshAnimTC [ m_pMesh->nTexLayerIDCount_ST ];
				if ( m_aAnimTC == NULL ) 
				{
					goto _ExitInitMeshWithError;
				}
			}

			// Init texture flipping palette...
			if ( m_pMesh->nTexLayerIDCount_Flip ) 
			{
				// This mesh has a texture flipping palette...

				m_aTexFlip = fnew CFMeshTexFlip [ m_pMesh->nTexLayerIDCount_Flip ];
				if ( m_aTexFlip == NULL ) 
				{
					goto _ExitInitMeshWithError;
				}
			}

			u32 nScrollIndex = 0;
			u32 nFlipIndex = 0;
			for ( i = 0; i < m_pMesh->nTexLayerIDCount; i++ ) 
			{
				m_aTexLayerID[i].nLayerAlpha = 255;
				m_aTexLayerID[i].pEmissiveMotif = NULL;

				if ( m_pMesh->pTexLayerIDArray[i].nFlags & FMESH_TEXLAYERIDFLAG_USE_ST_INFO ) 
				{
					FASSERT( nScrollIndex < m_pMesh->nTexLayerIDCount_ST );

					m_aAnimTC[nScrollIndex].m_bApplyToShader = TRUE;
					m_aAnimTC[nScrollIndex].m_nFlags =  (!!(m_pMesh->pTexLayerIDArray[i].nFlags & FMESH_TEXLAYERIDFLAG_BEGIN_SCROLLING)) * CFMeshAnimTC::SCROLL_APPLIED;
					m_aAnimTC[nScrollIndex].m_nFlags |= (!!(m_pMesh->pTexLayerIDArray[i].nFlags & FMESH_TEXLAYERIDFLAG_BEGIN_ROTATING)) * CFMeshAnimTC::ROTATE_APPLIED;
					m_aAnimTC[nScrollIndex].m_nFMeshLayerIndex = i;
					m_aAnimTC[nScrollIndex].m_fRotationRad = 0.f;
					m_aAnimTC[nScrollIndex].m_fUVRotationSpeed = FMATH_DEG2RAD( m_pMesh->pTexLayerIDArray[i].m_fUVDegreeRotationPerSec );
					m_aAnimTC[nScrollIndex].m_ScrollPos.x = 0.f;
					m_aAnimTC[nScrollIndex].m_ScrollPos.y = 0.f;
					m_aAnimTC[nScrollIndex].m_ScrollSTPerSec = m_pMesh->pTexLayerIDArray[i].m_ScrollSTPerSec;

					m_aTexLayerID[i].pAnimTC = &m_aAnimTC[nScrollIndex];

					nScrollIndex++;
				}
				if ( m_pMesh->pTexLayerIDArray[i].nFlags & FMESH_TEXLAYERIDFLAG_USE_FLIP_INFO ) 
				{
					FASSERT( nFlipIndex < m_pMesh->nTexLayerIDCount_Flip );

					m_aTexFlip[nFlipIndex].m_nFlags = CFMeshTexFlip::FLAG_APPLY_TO_SHADER;

					if( m_pMesh->pTexLayerIDArray[i].nFlags & FMESH_TEXLAYERIDFLAG_BEGIN_FLIPPING ) {
						FMATH_SETBITMASK( m_aTexFlip[nFlipIndex].m_nFlags, CFMeshTexFlip::FLAG_ANIMATE_FLIP );
					}

					m_aTexFlip[nFlipIndex].m_nFlipPageCount = m_pMesh->pTexLayerIDArray[i].nFlipPageCount;
					m_aTexFlip[nFlipIndex].m_nCurrentPageIndex = 0;
					m_aTexFlip[nFlipIndex].m_fCurrentPageIndex = 0.0f;
					m_aTexFlip[nFlipIndex].m_fPagesPerSecond = 60.0f / (f32)m_pMesh->pTexLayerIDArray[i].nFramesPerFlip;
					m_aTexFlip[nFlipIndex].m_apFlipPalette = m_pMesh->pTexLayerIDArray[i].apFlipPalette;
					if ( m_pMesh->pTexLayerIDArray[i].nFlipPageCount && m_pMesh->pTexLayerIDArray[i].apFlipPalette ) 
					{
						m_aTexFlip[nFlipIndex].m_pTexInst = m_pMesh->pTexLayerIDArray[i].apFlipPalette[ m_aTexFlip[nFlipIndex].m_nCurrentPageIndex ];
					} 
					else 
					{
						m_aTexFlip[nFlipIndex].m_pTexInst = NULL;
					}

					m_aTexLayerID[i].pTexFlip = &m_aTexFlip[nFlipIndex];

					nFlipIndex++;
				}
			}
		}

		if( !bNoMemAlloc )
		{
			BOOL bLiquidCreated=FALSE;
			for ( i = 0; i < m_pMesh->nMaterialCount; i++ ) 
			{
				if ( (m_pMesh->aMtl[i].nSurfaceShaderIdx >= FSHADERS_LIQUID_ENV && m_pMesh->aMtl[i].nSurfaceShaderIdx <= FSHADERS_LIQUID_MOLTEN_2LAYER) && !bLiquidCreated)
				{
					bLiquidCreated = TRUE;

					CFLiquidVolume *pLiquid = LiquidSystem.CreateLiquidVolume();
					
					if (pLiquid)
					{
						CFColorRGB white(1,1,1);
						LiquidType_e nType;
						nType = (m_pMesh->aMtl[i].nSurfaceShaderIdx<=FSHADERS_LIQUID_LAYER_ENV)?(LT_TEXTURE):(LT_MOLTEN);

						pLiquid->SetupProcedural(this);
						
						pLiquid->SetLiquidType(nType);
						pLiquid->SetLiquidFog(white, 0);
												
						pLiquid->Init();
						pLiquid->AddRainDrops(1, 2.0f);

						pLiquid->ChangeWaveSpeed((m_pMesh->aMtl[i].nSurfaceShaderIdx==FSHADERS_LIQUID_MOLTEN_2LAYER || m_pMesh->aMtl[i].nSurfaceShaderIdx==FSHADERS_LIQUID_MOLTEN_1LAYER)?(1.0f):(1.5f));

						m_pProcedural = pLiquid;
					}
				}
			}
		}
	}
	return TRUE;

_ExitInitMeshWithError:
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}


//
//
//
void CFMeshInst::SetLightmaps( cchar **pLightMapNameArray, const u16 *anLightMapMotif, u32 nArrayCount ) 
{
	u32 i, iLMCount = 0;
	for ( i = 0; i < nArrayCount; i++ )
	{
		if ( pLightMapNameArray[i] != NULL )
		{
			iLMCount++;
		}
	}

	if ( iLMCount == 0 )
	{
		return;
	}

	FResFrame_t Frame = fres_GetFrame();

	// We need to allocate enough space for the standard 13 registers, plus 3 more for each 
	// lightmap (FShTexInst_t pointer, uv index, and motif index)
	m_panLightRegisterLMOverride = (u32 *)fres_AllocAndZero( sizeof( u32 ) * (iLMCount * 3) + 1 );
	if ( !m_panLightRegisterLMOverride )
	{
		return;
	}

	m_panLightRegisterLMOverride[0] = 0;
	u32 *pLightmapRegister = &m_panLightRegisterLMOverride[1];
	for ( i = 0; i < nArrayCount; i++ )
	{
		if ( pLightMapNameArray[i] != NULL )
		{
			FTexDef_t *pTexDef = (FTexDef_t *)fresload_LoadStreaming( FTEX_RESNAME, pLightMapNameArray[i] );
//			FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, pLightMapNameArray[i] );
			if ( pTexDef == NULL )
			{
				DEVPRINTF( "fshaders::_FixupShTexInst() - Could not load lightmap texture '%s' for object '%s'. Removing static lighting.\n", pLightMapNameArray[i], m_pMesh->szName );
				goto __LOADFAILED;
			}
			(*pLightmapRegister) = (u32)fres_AllocAndZero( sizeof( FShTexInst_t ) );
			FShTexInst_t *pShTexInst = (FShTexInst_t *)(*pLightmapRegister);
			pShTexInst->TexInst.SetTexDef( pTexDef );
			pLightmapRegister++;
			(*pLightmapRegister) = 1;
			pLightmapRegister++;
			(*pLightmapRegister) = (u32)anLightMapMotif[i];
			pLightmapRegister++;
			m_panLightRegisterLMOverride[0]++;
		}
	}

	return;

__LOADFAILED:
	fres_ReleaseFrame( Frame );
	m_panLightRegisterLMOverride = NULL;
	m_nFlags &= ~(FMESHINST_FLAG_NOLIGHT_AMBIENT|FMESHINST_FLAG_NOLIGHT_DYNAMIC|FMESHINST_FLAG_LM|FMESHINST_FLAG_VERT_RADIOSITY);
}

//
//
//
void CFMeshInst::ResetLightList( void ) 
{
	m_nRenderLightCount = 0;
	
	if ( FMesh_nLightCount == FMesh_nMaxLightCount )
	{
		m_pRenderLights = NULL;
		return;
	}
	
	m_pRenderLights = &FMesh_paLightList[ FMesh_nLightCount ];
	m_nLowestWeightedLightIdx = -1;
}


//
//
//
BOOL CFMeshInst::ConsiderLightForRender( CFLight *pLight, CFSphere *pBoundingSphere_WS/*=NULL*/ ) 
{
	u32 i;
	f32 fLightWeight;
	CFSphere *pSphereWS=NULL;
	
	if ( !m_pRenderLights )
	{
		return FALSE;
	}

	// If a light is not attached to an object, then we need to consider whether it should
	// be applied to objects that have already received static lighting in the tools.
	if ( !(pLight->m_nFlags & FLIGHT_FLAG_LIGHT_ATTACHED) )
	{
		if ( (m_nFlags & FMESHINST_FLAG_LM) && (pLight->m_nFlags & (FLIGHT_FLAG_LIGHTMAP_LIGHT|FLIGHT_FLAG_DYNAMIC_ONLY)) )
		{
			// This mesh has lightmaps and the light was used in the lightmapping phase
			return FALSE;
		}

		if ( (m_nFlags & FMESHINST_FLAG_VERT_RADIOSITY) && (pLight->m_nFlags & (FLIGHT_FLAG_LIGHTMAP_LIGHT|FLIGHT_FLAG_DYNAMIC_ONLY)) && pLight->m_Motif.nMotifIndex == 0 )
		{
			// This mesh has been lit with vertex radiosity and thus should only receive
			// dynamic lights or static lights that have a motif
			return FALSE;
		}
	}

	if ( pLight->m_nFlags & FLIGHT_FLAG_CORONA_ONLY )
	{
		//this light is only used to display a corona, do not consider it!
		return FALSE;
	}

	if ( !(pLight->m_nFlags & FLIGHT_FLAG_ENABLE) )
	{
		//this light is not enabled.
		return FALSE;
	}

	if ( pLight->m_pOwnerMeshInst == (const CFMeshInst *)this ) 
	{
		// The mesh owns the light...

		if ( !(pLight->m_nFlags & FLIGHT_FLAG_LIGHT_ATTACHED) ) 
		{
			// Light is not permitted to light its owner...
			return FALSE;
		}
	} 
	else 
	{
		// The mesh does not own the light...

		if ( pLight->m_pOwnerMeshInst && (pLight->m_nFlags & FLIGHT_FLAG_DONT_LIGHT_UNATTACHED) ) 
		{
			// Light is not permitted to light non-owners...
			return FALSE;
		}
	}

	if ( pBoundingSphere_WS )
	{
		pSphereWS = pBoundingSphere_WS;
	}
	else
	{
		pSphereWS = &m_BoundSphere_MS;
	}

	// Calculate the weight of this light for this mesh
	if ( pLight->m_nFlags & FLIGHT_FLAG_GAMEPLAY_LIGHT || pLight->m_nType == FLIGHT_TYPE_DIR )
	{
		fLightWeight = 3.f;
	}
	else
	{
		fLightWeight = pLight->GetWeight( pSphereWS );
	}
	
	if ( fLightWeight < 0.00001f )
	{
		return FALSE;
	}
	
#if !FANG_PRODUCTION_BUILD
	if ( this == FMesh_PlayerMesh && FPerf_nDisplayPerfType == FPERF_TYPE_LIGHTING )
	{
		_nTotalLights++;
		fperf_Render_AddPerfInt( FPERF_TYPE_LIGHTING, "Lights :", _nTotalLights, 0.f );
		char szTemp[64];
		if ( pLight->m_nFlags & FLIGHT_FLAG_HASDIR )
		{
			if ( pLight->m_nFlags & FLIGHT_FLAG_HASPOS )
			{
				sprintf( szTemp, "S Light%02d:", _nTotalLights );
			}
			else
			{
				sprintf( szTemp, "D Light%02d:", _nTotalLights );
			}
		}
		else
		{
			sprintf( szTemp, "O Light%02d:", _nTotalLights );
		}

		fperf_Render_AddPerfFloat( FPERF_TYPE_LIGHTING, szTemp, fLightWeight, 0.f );

		CFColorRGBA Color;

		Color.Set( pLight->m_ScaledColor.fRed, pLight->m_ScaledColor.fGreen, pLight->m_ScaledColor.fBlue, 1.f );
		fdraw_DevSphere( &pLight->m_spInfluence_WS.m_Pos, 0.4f, &Color );

		if ( pBoundingSphere_WS )
		{
			fdraw_DevLine( &pLight->m_spInfluence_WS.m_Pos, &pBoundingSphere_WS->m_Pos, &Color, &Color );
		}
		else
		{
			fdraw_DevLine( &pLight->m_spInfluence_WS.m_Pos, &m_BoundSphere_MS.m_Pos, &Color, &Color );
		}
	}
#endif

	// Make sure we don't use more lights than are allowable
	if ( m_nRenderLightCount == FMesh_nMaxMeshLights || FMesh_nLightCount == FMesh_nMaxLightCount )
	{
#if !FANG_PRODUCTION_BUILD	
		if ( FMesh_nLightCount == FMesh_nMaxLightCount && (FVid_nFrameCounter - _nBufferWarningFrame) > 600 )
		{
			_nBufferWarningFrame = FVid_nFrameCounter;
			DEVPRINTF( "CFMeshInst::ConsiderLightForRender() - Reached _ppDrawLightBuffer[] buffer limit for mesh lights.  Current buffer size: %d.\n", FMesh_nMaxLightCount );
		}
#endif

		// If the submitted light's weight is already lower than the lowest, then we can trivially reject
		if ( fLightWeight < m_pRenderLights[m_nLowestWeightedLightIdx].fLightWeight && !(pLight->m_nFlags&FLIGHT_FLAG_GAMEPLAY_LIGHT) )
		{
			return FALSE;
		}
		
		// Replace the lowest
		m_pRenderLights[m_nLowestWeightedLightIdx].pLight = pLight;
		m_pRenderLights[m_nLowestWeightedLightIdx].fLightWeight = fLightWeight;

		// Calculate the new lowest
		f32 fLowestWeight = FMATH_MAX_FLOAT;
		for ( i = 0; i < m_nRenderLightCount; i++ )
		{
			if ( m_pRenderLights[i].fLightWeight < fLowestWeight && !(m_pRenderLights[i].pLight->m_nFlags&FLIGHT_FLAG_GAMEPLAY_LIGHT) )
			{
				m_nLowestWeightedLightIdx = i;
				fLowestWeight = m_pRenderLights[i].fLightWeight;
			}
		}
	}
	else
	{
		// Add this light to the list
		m_pRenderLights[m_nRenderLightCount].pLight = pLight;
		m_pRenderLights[m_nRenderLightCount].fLightWeight = fLightWeight;

		// See if it is the lowest weight
		if ( (m_nLowestWeightedLightIdx == -1 || fLightWeight < m_pRenderLights[m_nLowestWeightedLightIdx].fLightWeight) && !(pLight->m_nFlags&FLIGHT_FLAG_GAMEPLAY_LIGHT) )
		{
			m_nLowestWeightedLightIdx = m_nRenderLightCount;
		}
		
		m_nRenderLightCount++;
		FMesh_nLightCount++;
	}

	//jjc
	if ( !FVis_bInRenderTarget && (pLight->m_nFlags & FLIGHT_FLAG_CAST_SHADOWS) && (m_nFlags & FMESHINST_FLAG_CAST_SHADOWS) )
	{
		FASSERT( pLight->m_nMaxShadowCount > 0 && pLight->m_papShadowMeshInst && pLight->m_paShadowBoundingSphere && pLight->m_pafShadowObjDist );
		
		u32 nShadows = fvis_GetNumShadows( pLight->m_nIndex );
		u32 nShIdx;
					
		CFVec3A vOffs;
		vOffs.Sub( m_Xfm.m_MtxF.m_vPos, FXFM_CAM_ORIG_WS );
		f32 fDist2 = vOffs.MagSq();
					
		if ( nShadows < pLight->m_nMaxShadowCount )
		{
			BOOL bAdd=TRUE;
			if (FShadow_bMultiplayer)
			{
				for (u32 i=0; i<nShadows; i++)
				{
					if (pLight->m_papShadowMeshInst[i] == this)
					{
						bAdd=FALSE;
						break;
					}
				}
			}
			if (bAdd)
			{
				pLight->m_pafShadowObjDist[nShadows] = fDist2;
				pLight->m_paShadowBoundingSphere[nShadows] = *pSphereWS;
				pLight->m_papShadowMeshInst[nShadows++] = this;
				fvis_SetNumShadows(pLight->m_nIndex, nShadows);
			}
		}
		else
		{
			for ( nShIdx=0; nShIdx < nShadows; nShIdx++ )
			{
				if ( fDist2 < pLight->m_pafShadowObjDist[nShIdx] )
				{
					pLight->m_papShadowMeshInst[nShIdx] = this;
					pLight->m_paShadowBoundingSphere[nShIdx] = *pSphereWS;
					pLight->m_pafShadowObjDist[nShIdx] = fDist2;
				}
			}
		}
	}
	//
	
	return TRUE;
}


//
//
//
void CFMeshInst::AnimateLayers( void ) 
{
	CFMeshAnimTC *pAnimTC;
	CFMeshTexFlip *pTexFlip;
	f32 fElapsedSecs;
	u32 i;

	if ( m_nFrameOfLastAnimateLayer == FVid_nFrameCounter ) 
	{
		// Same frame, so don't animate layers again...
		return;
	}

	m_nFrameOfLastAnimateLayer = FVid_nFrameCounter;

	if ( m_pMesh->nTexLayerIDCount ) 
	{
		// Compute elapsed time since last layer animation...

		if ( !(m_nFlags & FMESHINST_FLAG_SAMPLE_ANIMLAYER_TICKS) && (m_nLastAnimateLayerTicks <= FLoop_nTotalLoopTicks) ) 
		{
			if ( !(m_nFlags & FMESHINST_FLAG_ANIM_WHEN_PAUSED) ) 
			{
				fElapsedSecs = (f32)(FLoop_nTotalLoopTicks - m_nLastAnimateLayerTicks) * FLoop_fSecsPerTick;
				m_nLastAnimateLayerTicks = FLoop_nTotalLoopTicks;
			} 
			else 
			{
				fElapsedSecs = (f32)(FLoop_nRealTotalLoopTicks - m_nLastAnimateLayerTicks) * FLoop_fSecsPerTick;
				m_nLastAnimateLayerTicks = FLoop_nRealTotalLoopTicks;
			}
		} 
		else 
		{
			FMATH_CLEARBITMASK( m_nFlags, FMESHINST_FLAG_SAMPLE_ANIMLAYER_TICKS );
			fElapsedSecs = 0.0f;

			if ( !(m_nFlags & FMESHINST_FLAG_ANIM_WHEN_PAUSED) ) 
			{
				m_nLastAnimateLayerTicks = FLoop_nTotalLoopTicks;
			} 
			else 
			{
				m_nLastAnimateLayerTicks = FLoop_nRealTotalLoopTicks;
			}
		}
	}

	//------------------------------------------------------------
	// TC Animations:

	if ( m_pMesh->nTexLayerIDCount_ST ) 
	{
		// There are TC animations for this model...

		// Step through and animate TCs...
		for ( i = 0, pAnimTC = m_aAnimTC; i < m_pMesh->nTexLayerIDCount_ST; i++, pAnimTC++ ) 
		{
			if ( pAnimTC->m_nFlags & CFMeshAnimTC::ROTATE_APPLIED ) 
			{
				pAnimTC->m_fRotationRad += pAnimTC->m_fUVRotationSpeed * fElapsedSecs;
				while ( pAnimTC->m_fRotationRad > FMATH_2PI )
				{
					pAnimTC->m_fRotationRad -= FMATH_2PI;
				}
				while ( pAnimTC->m_fRotationRad < -FMATH_2PI )
				{
					pAnimTC->m_fRotationRad += FMATH_2PI;
				}
			}
			if ( pAnimTC->m_nFlags & CFMeshAnimTC::SCROLL_APPLIED ) 
			{
				pAnimTC->m_ScrollPos += pAnimTC->m_ScrollSTPerSec * fElapsedSecs;
				NormalizeScrollST( pAnimTC );
			}
		}
	}

	//------------------------------------------------------------
	// Texture Flip Animations:

	if ( m_pMesh->nTexLayerIDCount_Flip ) 
	{
		// There are texture flipping animations for this model...

		// Step through and flip textures...
		for ( i = 0, pTexFlip = m_aTexFlip; i < m_pMesh->nTexLayerIDCount_Flip; i++, pTexFlip++ ) 
		{
			if ( (pTexFlip->m_nFlags & CFMeshTexFlip::FLAG_ANIMATE_FLIP) && pTexFlip->m_apFlipPalette && (pTexFlip->m_nFlipPageCount > 1) ) 
			{
				// Flipping is enabled...

				pTexFlip->m_fCurrentPageIndex += pTexFlip->m_fPagesPerSecond * fElapsedSecs;

				if ( !(pTexFlip->m_nFlags & CFMeshTexFlip::FLAG_CLAMP_AND_STOP) )
				{
					pTexFlip->m_nCurrentPageIndex = (u8)( (u32)pTexFlip->m_fCurrentPageIndex % (u32)pTexFlip->m_nFlipPageCount );
				}
				else
				{
					pTexFlip->m_nCurrentPageIndex = (u8)pTexFlip->m_fCurrentPageIndex;

					if ( pTexFlip->m_fPagesPerSecond > 0.f )
					{
						if ( pTexFlip->m_fCurrentPageIndex >= (f32)(pTexFlip->m_nFlipPageCount - 1) )
						{
							pTexFlip->m_nCurrentPageIndex = pTexFlip->m_nFlipPageCount - 1;
							pTexFlip->m_fCurrentPageIndex = (f32)pTexFlip->m_nCurrentPageIndex;

							TexFlip_AnimateFlip( (FMeshTexLayerHandle_t)i, FALSE );
						}
					}
					else if ( pTexFlip->m_fPagesPerSecond < 0.f )
					{
						if ( pTexFlip->m_fCurrentPageIndex <= 0.f )
						{
							pTexFlip->m_nCurrentPageIndex = 0;
							pTexFlip->m_fCurrentPageIndex = 0.f;

							TexFlip_AnimateFlip( (FMeshTexLayerHandle_t)i, FALSE );
						}
					}
				}

				pTexFlip->m_fCurrentPageIndex = (f32)pTexFlip->m_nCurrentPageIndex + (pTexFlip->m_fCurrentPageIndex - (f32)((u32)pTexFlip->m_fCurrentPageIndex) );

				pTexFlip->m_pTexInst = pTexFlip->m_apFlipPalette[ pTexFlip->m_nCurrentPageIndex ];
			}
		}
	}
}


//
//
//
void CFMeshInst::NormalizeScrollValueUp( f32 *pfValue ) 
{
	FASSERT( *pfValue <= -1.0f );

	if ( *pfValue < -_MAX_TC_VALUE_TO_ADJUST_BY_LOOPING ) 
	{
		// Compute modulo without looping...
		*pfValue = fmath_Fmod( *pfValue, -1.0f );
	} 
	else 
	{
		// Compute module with looping...
		while( *pfValue <= -1.0f ) 
		{
			*pfValue += 1.0f;
		}
	}
}


//
//
//
void CFMeshInst::NormalizeScrollValueDown( f32 *pfValue ) 
{
	FASSERT( *pfValue >= 1.0f );

	if ( *pfValue > _MAX_TC_VALUE_TO_ADJUST_BY_LOOPING ) 
	{
		// Compute modulo without looping...
		*pfValue = fmath_Fmod( *pfValue, 1.0f );
	} 
	else 
	{
		// Compute module with looping...
		while( *pfValue >= 1.0f ) 
		{
			*pfValue -= 1.0f;
		}
	}
}


//
//
//
f32 CFMeshInst::GetMatAngularIntensity( FMeshMaterial_t *pMat ) const 
{
	// Determine angle to camera
	CFVec3A vUncompAffectNormal, vAvgPos( pMat->vAverageVertPos );
	vUncompAffectNormal.Set( pMat->anCompAffectNormal[0] * (1.f/64.f), pMat->anCompAffectNormal[1] * (1.f/64.f), pMat->anCompAffectNormal[2] * (1.f/64.f) );
	if ( pMat->nAffectBoneID == -1 || m_pMesh->nUsedBoneCount == 0 )
	{
		m_Xfm.m_MtxF.MulDir( vUncompAffectNormal );
		m_Xfm.m_MtxF.MulPoint( vAvgPos );
		vUncompAffectNormal.Mul( m_Xfm.m_fScaleR );
	}
	else 
	{
		FASSERT( m_apBoneMtxPalette );
		if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) ) 
		{
			m_apBoneMtxPalette[ pMat->nAffectBoneID ]->MulDir( vUncompAffectNormal );
			m_apBoneMtxPalette[ pMat->nAffectBoneID ]->MulPoint( vAvgPos );
			f32 fInvScale = m_apBoneMtxPalette[ pMat->nAffectBoneID ]->m_vFront.MagSq();
			if ( fInvScale > 0.001f )
			{
				vUncompAffectNormal.Mul( fmath_InvSqrt( fInvScale ) );
			}
		}
		else
		{
			CFMtx43A Mtx43A;
			Mtx43A.Mul( m_Xfm.m_MtxF, m_pMesh->pBoneArray[ pMat->nAffectBoneID ].AtRestBoneToModelMtx );
			Mtx43A.MulDir( vUncompAffectNormal );
			Mtx43A.MulPoint( vAvgPos );
			f32 fInvScale = Mtx43A.m_vFront.MagSq();
			if ( fInvScale > 0.001f )
			{
				vUncompAffectNormal.Mul( fmath_InvSqrt( fInvScale ) );
			}
		}
	}

	f32 fLengthSq = vAvgPos.Sub( FXfm_pView->m_MtxR.m_vPos ).MagSq();
	if ( fLengthSq > 0.001f )
	{
		vAvgPos.Mul( fmath_InvSqrt( fLengthSq ) );
		f32 fCosAngle = -vAvgPos.Dot( vUncompAffectNormal );
		if ( fCosAngle > pMat->fAffectAngle )
		{
			return fmath_Div( fCosAngle - pMat->fAffectAngle, 1.f - pMat->fAffectAngle );
		}
	}

	return 0.f;
}


//
//
//
BOOL CFMeshInst::SetupShaderParameters( FMeshMaterial_t *pMaterial, u32 nPasses ) const 
{
	u32 i, nTexLayerIDIndex;
	CFColorMotif *pEmissiveOverride = NULL;

	if ( nPasses & FSHADERS_PASS_LIGHTING )
	{
		// Handle angular emissive
		if ( pMaterial->nMtlFlags & FMESH_MTLFLAG_ANGULAR_EMISSIVE )
		{
			f32 fIntensity = GetMatAngularIntensity( pMaterial );
			_AngularEmissive.ColorRGB.Set( fIntensity, fIntensity, fIntensity );
			pEmissiveOverride = &_AngularEmissive;
		}
		
#if !FANG_PLATFORM_PS2	//ARG
		// Setup texture layer effects
		if ( m_pMesh->nTexLayerIDCount ) 
		{
			const FMeshTexLayerID_t *aTexLayerID = m_pMesh->pTexLayerIDArray;

			fsh_TexCoordMtx_ResetList();

			for ( i=0; (nTexLayerIDIndex = pMaterial->anTexLayerIDIndex[i])!=255; i++ ) 
			{
				FMeshInstTexLayerID_t *pLayerID = GetTexLayerID( nTexLayerIDIndex );

				if ( pLayerID->pAnimTC && pLayerID->pAnimTC->m_bApplyToShader && !(pMaterial->nMtlFlags & FMESH_MTLFLAG_NO_ALPHA_SCROLL) ) 
				{
					// Calculate the texture matrix
					f32 fAnchorX = m_pMesh->pTexLayerIDArray[pLayerID->pAnimTC->m_nFMeshLayerIndex].m_nCompressedUVRotAnchor[0] * (1.f/255.f);
					f32 fAnchorY = m_pMesh->pTexLayerIDArray[pLayerID->pAnimTC->m_nFMeshLayerIndex].m_nCompressedUVRotAnchor[1] * (1.f/255.f);

					_TextureMtx[i].aa[0][0] = 1.f;
					_TextureMtx[i].aa[0][1] = 0.f;
					_TextureMtx[i].aa[1][0] = 0.f;  
					_TextureMtx[i].aa[1][1] = 1.f;
					_TextureMtx[i].aa[2][0] = -fAnchorX;
					_TextureMtx[i].aa[2][1] = -fAnchorY;
					_TextureMtx[i].RotateZ( pLayerID->pAnimTC->m_fRotationRad );
					_TextureMtx[i].aa[2][0] += fAnchorX + pLayerID->pAnimTC->m_ScrollPos.x;
					_TextureMtx[i].aa[2][1] += fAnchorY + pLayerID->pAnimTC->m_ScrollPos.y;

					fsh_TexCoordMtx_Add( aTexLayerID[nTexLayerIDIndex].nTexLayerID, &_TextureMtx[i] );
				}

				if( pLayerID->pTexFlip && (pLayerID->pTexFlip->m_nFlags & CFMeshTexFlip::FLAG_APPLY_TO_SHADER) ) 
				{
					fsh_TexOverride_Add( aTexLayerID[nTexLayerIDIndex].nTexLayerID, pLayerID->pTexFlip->m_pTexInst );
				}

				// Track the programmer set emissive
				if ( pLayerID->pEmissiveMotif )
				{
					pEmissiveOverride = pLayerID->pEmissiveMotif;
				}
			}
		}
#endif	//ARG
		
		fsh_ZWriteEnable( !!(pMaterial->nMtlFlags & FMESH_MTLFLAG_ZWRITE_ON) );
		fsh_SetInvAlphaToggle_Emissive( !!(pMaterial->nMtlFlags & FMESH_MTLFLAG_INVERT_EMISSIVE_MASK) );
		
		// Setup shader mesh tinting
		if ( pMaterial->nMtlFlags & FMESH_MTLFLAG_DO_NOT_TINT )
		{
			fsh_SetSurfaceColor( pMaterial->MaterialTint.fRed, pMaterial->MaterialTint.fGreen, pMaterial->MaterialTint.fBlue, m_MeshRGBA.fAlpha );
		}
		else
		{
			fsh_SetSurfaceColor( pMaterial->MaterialTint.fRed * m_MeshRGBA.fRed, pMaterial->MaterialTint.fGreen * m_MeshRGBA.fGreen, pMaterial->MaterialTint.fBlue * m_MeshRGBA.fBlue, m_MeshRGBA.fAlpha );
		}
	}

	if ( nPasses & FSHADERS_PASS_SURFACE )
	{
		f32 fLayerAlpha = 1.f;

		// Setup texture layer effects
		if ( m_pMesh->nTexLayerIDCount ) 
		{
			const FMeshTexLayerID_t *aTexLayerID = m_pMesh->pTexLayerIDArray;

			fsh_TexCoordMtx_ResetList();
			fsh_TexOverride_ResetList();

			for ( i=0; (nTexLayerIDIndex = pMaterial->anTexLayerIDIndex[i])!=255; i++ ) 
			{
				FMeshInstTexLayerID_t *pLayerID = GetTexLayerID(nTexLayerIDIndex);

				if ( pLayerID->pAnimTC && pLayerID->pAnimTC->m_bApplyToShader ) 
				{
					// Calculate the texture matrix
					f32 fAnchorX = m_pMesh->pTexLayerIDArray[pLayerID->pAnimTC->m_nFMeshLayerIndex].m_nCompressedUVRotAnchor[0] * (1.f/255.f);
					f32 fAnchorY = m_pMesh->pTexLayerIDArray[pLayerID->pAnimTC->m_nFMeshLayerIndex].m_nCompressedUVRotAnchor[1] * (1.f/255.f);

					_TextureMtx[i].aa[0][0] = 1.f;
					_TextureMtx[i].aa[0][1] = 0.f;
					_TextureMtx[i].aa[1][0] = 0.f;  
					_TextureMtx[i].aa[1][1] = 1.f;
					_TextureMtx[i].aa[2][0] = -fAnchorX;
					_TextureMtx[i].aa[2][1] = -fAnchorY;
					_TextureMtx[i].RotateZ( pLayerID->pAnimTC->m_fRotationRad );
					_TextureMtx[i].aa[2][0] += fAnchorX + pLayerID->pAnimTC->m_ScrollPos.x;
					_TextureMtx[i].aa[2][1] += fAnchorY + pLayerID->pAnimTC->m_ScrollPos.y;

					fsh_TexCoordMtx_Add( aTexLayerID[nTexLayerIDIndex].nTexLayerID, &_TextureMtx[i] );
				}

				if( pLayerID->pTexFlip && (pLayerID->pTexFlip->m_nFlags & CFMeshTexFlip::FLAG_APPLY_TO_SHADER) ) 
				{
					fsh_TexOverride_Add( aTexLayerID[nTexLayerIDIndex].nTexLayerID, pLayerID->pTexFlip->m_pTexInst );
				}

				// Track the programmer set layer alpha (note that we allow this to override angular alpha
				fLayerAlpha = LayerAlpha_Get((FMeshTexLayerHandle_t)nTexLayerIDIndex);

//ARG - >>>>>
#if FANG_PLATFORM_PS2
				// Track the programmer set emissive
				if ( pLayerID->pEmissiveMotif )
				{
					pEmissiveOverride = pLayerID->pEmissiveMotif;
				}
#endif
//ARG - <<<<<
			}
		}
		
		// Determine appropriate layer alpha
		if ( pMaterial->nMtlFlags & FMESH_MTLFLAG_ANGULAR_TRANSLUCENCY )
		{
			fLayerAlpha = GetMatAngularIntensity( pMaterial );
			if ( fLayerAlpha < 0.01f )
			{
				return FALSE;
			}
		}

		fsh_SetLayerAlpha( fLayerAlpha );
		fsh_SetReflectTex( m_pReflectTex );
		
		// Setup shader mesh tinting
		if ( pMaterial->nMtlFlags & FMESH_MTLFLAG_DO_NOT_TINT )
		{
			fsh_SetSurfaceColor( pMaterial->MaterialTint.fRed, pMaterial->MaterialTint.fGreen, pMaterial->MaterialTint.fBlue, m_MeshRGBA.fAlpha );
		}
		else
		{
			fsh_SetSurfaceColor( pMaterial->MaterialTint.fRed * m_MeshRGBA.fRed, pMaterial->MaterialTint.fGreen * m_MeshRGBA.fGreen, pMaterial->MaterialTint.fBlue * m_MeshRGBA.fBlue, m_MeshRGBA.fAlpha );
		}
	}

	fsh_SetDepthBias( pMaterial->nDepthBiasLevel );

	// Set shader registers
	u32 *pnLightmapRegisters = NULL;
#if !FANG_PLATFORM_PS2	//ARG
	if ( m_panLightRegisterLMOverride && pMaterial->pnShLightRegisters[FSHADERS_LIGHT_REG_LMCOUNT] )
	{
		// We have lightmap overrides so setup this instance's lightmap registers
		for ( i = 0; i < m_panLightRegisterLMOverride[0]; i++ )
		{
			// Make sure our ST indices are correct (could be different for different materials)
			// All of the lightmap TC's are the same as the first TC set
			m_panLightRegisterLMOverride[(FSHADERS_LIGHT_REG_LM_TC - FSHADERS_LIGHT_REG_LMCOUNT) + (i*3)] = pMaterial->pnShLightRegisters[FSHADERS_LIGHT_REG_LM_TC];
		}

		// Set the lightmap register overrides
		pnLightmapRegisters = m_panLightRegisterLMOverride;
	}
	else if ( (m_nFlags & FMESHINST_FLAG_LM) || (m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH) )
	{
		// Use the existing lightmap registers
		pnLightmapRegisters = &pMaterial->pnShLightRegisters[FSHADERS_LIGHT_REG_LMCOUNT];
	}
#endif	//ARG

	fsh_SetupShaderRegisters( pMaterial->pnShLightRegisters, pMaterial->pnShSurfaceRegisters, pnLightmapRegisters, pEmissiveOverride );
	fsh_SetSurfaceShader( pMaterial->nSurfaceShaderIdx );

	return TRUE;
}


//
//
//
s32 CFMeshInst::FindBone( cchar *pszBoneName ) const 
{
	if( pszBoneName == NULL )
	{
		return -1;
	}

	u32 i;

	FASSERT( _bModuleInitialized );
	FASSERT( pszBoneName );

	for ( i = 0; i < m_pMesh->nBoneCount; i++ ) 
	{
		if( !fclib_stricmp( pszBoneName, m_pMesh->pBoneArray[i].szName ) ) 
		{
			return (s32)i;
		}
	}

	return -1;
}


//
//
//
void CFMeshInst::AtRestMtxPalette( void )
{
	FASSERT( m_pMesh != NULL );

	u32 i;

	for( i = 0; i < m_pMesh->nBoneCount; i++ )
	{
		if( m_pMesh->pBoneArray[i].nFlags & FMESH_BONEFLAG_SKINNEDBONE )
		{
			*m_apBoneMtxPalette[i] = m_Xfm.m_MtxF;
		}
		else
		{
			m_apBoneMtxPalette[i]->Mul( m_Xfm.m_MtxF, m_pMesh->pBoneArray[i].AtRestBoneToModelMtx );
		}
	}
}


//
//
//
FMeshTexLayerHandle_t CFMeshInst::GetTexLayerHandle( u32 nTexLayerID ) const 
{
	u32 i;

	FASSERT( _bModuleInitialized );
	FASSERT( m_pMesh );

	for ( i = 0; i < m_pMesh->nTexLayerIDCount; i++ ) 
	{
		if ( m_pMesh->pTexLayerIDArray[i].nTexLayerID == nTexLayerID ) 
		{
			return (FMeshTexLayerHandle_t)i;
		}
	}

	return FMESH_TEXLAYERHANDLE_INVALID;
}


//
//
//
FViewportPlanesMask_t CFMeshInst::DrawPrep( FViewportPlanesMask_t nCrossesPlanesMask, BOOL bUnconditional/*=FALSE*/, BOOL bDrawFull/*=TRUE*/ ) 
{
	FViewportPlanesMask_t nCrossesPlanesMaskToUse;
	CFSphere *pMeshBoundSphere_WS, MeshBoundSphere_WS;
	FViewport_t *pActiveViewport;
	CFVec3A TempVec3A;

	FASSERT( _bModuleInitialized );

#if FMESH_COLL_ENABLE_COLLISION_DRAWS
	if ( FMesh_nDrawCollisionGeoFlags )
	{
		fmesh_Coll_DrawCollGeo( this, FMesh_nDrawCollisionGeoFlags );
		return nCrossesPlanesMask;
	}
#endif

	if ( !bUnconditional )
	{
		pActiveViewport = fviewport_GetActive();
		if ( pActiveViewport == NULL ) 
		{
			// No viewport. Consider object entirely culled...
			return -1;
		}

		if ( m_nFlags & FMESHINST_FLAG_DONT_DRAW ) 
		{
			// Don't draw this instance...
			return -1;
		}

		if ( nCrossesPlanesMask == -1 ) 
		{
			// Instance is completely outside frustum...
			return -1;
		}

		TempVec3A.Sub( FXfm_pView->m_MtxR.m_vPos, m_Xfm.m_MtxF.m_vPos );
		if ( m_fCullDist!=0.0f && TempVec3A.MagSq() > m_fCullDist * m_fCullDist ) 
		{
			// Object is too far to draw...
			return -1;
		}
	}
	
	FASSERT( nCrossesPlanesMask != -1 );

	if ( !bUnconditional )
	{
		if ( m_nFlags & FMESHINST_FLAG_WORLDSPACE ) 
		{
			// Mesh is already in world space...
			pMeshBoundSphere_WS = &m_BoundSphere_MS;
		} 
		else 
		{
			// Mesh is not in world space. Let's transform its bound sphere into world space...
			FXfm_pModel->TransformSphereF( MeshBoundSphere_WS, m_BoundSphere_MS );
			pMeshBoundSphere_WS = &MeshBoundSphere_WS;
		}
		
		nCrossesPlanesMask = fviewport_TestSphere_WS( pActiveViewport, pMeshBoundSphere_WS, nCrossesPlanesMask );
		if ( nCrossesPlanesMask == -1 ) 
		{
			// Mesh is completely outside the frustum...
//			CFXfm::PopModel( nNumPops );
			return -1;
		}

		// If the no-clip flag is set, use 0 for the crosses planes mask...
		nCrossesPlanesMaskToUse = nCrossesPlanesMask;
		if ( m_nFlags & FMESHINST_FLAG_NOCLIP ) 
		{
			nCrossesPlanesMaskToUse = 0;
		}
	}

#if !FANG_PRODUCTION_BUILD
/*
	if ( m_pMesh->nCollTreeCount )
	{
		u32 i;
		frenderer_Push( FRENDERER_DRAW, NULL );
		for ( i = 0; i < m_pMesh->nCollTreeCount; i++ )
		{
			fdraw_FacetedWireSphere( &m_pMesh->paCollTree[i].BoundingCapsule.vStart, m_pMesh->paCollTree[i].BoundingCapsule.fRadius, 2, 2 );
			fdraw_FacetedWireSphere( &m_pMesh->paCollTree[i].BoundingCapsule.vEnd, m_pMesh->paCollTree[i].BoundingCapsule.fRadius, 2, 2 );
		}
		fdraw_ModelSpaceAxis( 0.5f * m_BoundSphere_MS.m_fRadius );
		frenderer_Pop();
	}
*/
	if ( FRenderer_bDrawBoundInfo )
	{
		u32 nNumPops = PushXfm();
		frenderer_Push( FRENDERER_DRAW, NULL );
		fdraw_FacetedWireSphere( &m_BoundSphere_MS.m_Pos, m_BoundSphere_MS.m_fRadius, 2, 2 );
		fdraw_ModelSpaceAxis( 0.5f * m_BoundSphere_MS.m_fRadius );
		frenderer_Pop();
		CFXfm::PopModel( nNumPops );
	}
#endif		

	AnimateLayers();

	#if FANG_WINGC
	if( m_pDrawFilter && !m_pDrawFilter( this, m_pDrawFilterData ) ) return -1;
	#endif

	// If a pre-draw callback has been specified, call it
	if ( m_pPreDrawCallback )
	{
		m_pPreDrawCallback( this, m_pPreDrawData );
	}

	if ( bDrawFull )
	{
//		nNumPops = PushXfm();

		// Instantly prep and draw the mesh
		if ( !DrawPrep_P( TRUE ) )
		{
			nCrossesPlanesMask = -1;
		}
		else
		{
			FASSERT( frenderer_GetActive() == FRENDERER_MESH );
			DrawAllMaterials_P( nCrossesPlanesMask );

			#if !FANG_PRODUCTION_BUILD
				if ( FRenderer_bDrawBoneInfo )
				{
					// Get back into world space...
					CFXfm Xfm;
					Xfm.ReceiveInverseOf( *FXfm_pModel );
					Xfm.PushModel();

					DrawBoneInfo();

					CFXfm::PopModel();
				}
			#endif
		}

//		CFXfm::PopModel( nNumPops );
	}
	else
	{
		// Do all of the material list setup here
		if ( !DrawPrep_P( FALSE ) )
		{
			return -1;
		}
	}
	
	// If a post-draw callback has been specified, call it
	if ( m_pPostDrawCallback )
	{
		m_pPostDrawCallback( this, m_pPostDrawData );
	}

	#if FPERF_ENABLE
	if ( m_nFlags & FMESHINST_FLAG_WORLD_GEO )
	{
		FPerf_nVolumeMeshDrawnCount++;
	}
	else
	{
		FPerf_nMeshInstDrawnCount++;
	}
	#endif
	
	FlagAsDrawnThisFrame();

	return nCrossesPlanesMask;
}


#if !FANG_PRODUCTION_BUILD
void CFMeshInst::DrawBoneInfo( void )
{
	if ( m_apBoneMtxPalette )
	{
		const CFMtx43A *pBoneMtx;
		u32 nBoneIndex;
		CFXfm Xfm;

		frenderer_Push( FRENDERER_DRAW, NULL );
		fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );

		for ( nBoneIndex = 0; nBoneIndex < m_pMesh->nBoneCount; ++nBoneIndex )
		{
			pBoneMtx = m_apBoneMtxPalette[ nBoneIndex ];
			Xfm.BuildFromMtx( *pBoneMtx, FALSE );
			Xfm.PushModel();

			fdraw_ModelSpaceAxis( 0.3f );

			CFXfm::PopModel();
		}

		frenderer_Pop();
	}
}
#endif


//
// Returns the number of times to pop.
//
u32 CFMeshInst::PushXfm( void ) const 
{
	f32 fCamX_MS, fCamY_MS, fCamZ_MS, fCamR_MS, fCamOOR_MS, fSin, fCos;
	u32 nNumPops;

	FASSERT( _bModuleInitialized );

	m_Xfm.PushModel();
	nNumPops = 1;

	if ( m_nFlags & FMESHINST_FLAG_POSTER_Y ) 
	{
		// Object is to be postered around its Y axis toward camera...

		fCamX_MS = FXfm_pModelView->m_MtxR.m_vPos.x;
		fCamZ_MS = FXfm_pModelView->m_MtxR.m_vPos.z;
		fCamR_MS = fmath_Sqrt( fCamX_MS*fCamX_MS + fCamZ_MS*fCamZ_MS );

		if ( fCamR_MS ) 
		{
			fCamOOR_MS = 1.0f / fCamR_MS;

			fSin = fCamOOR_MS * fCamX_MS;
			fCos = fCamOOR_MS * fCamZ_MS;

			_XfmPosterY.m_MtxF.aa[0][0] = _XfmPosterY.m_MtxF.aa[2][2] =  fCos;
			_XfmPosterY.m_MtxR.aa[0][0] = _XfmPosterY.m_MtxR.aa[2][2] =  fCos;
			_XfmPosterY.m_MtxF.aa[0][2] = _XfmPosterY.m_MtxR.aa[2][0] = -fSin;
			_XfmPosterY.m_MtxF.aa[2][0] = _XfmPosterY.m_MtxR.aa[0][2] =  fSin;

			_XfmPosterY.PushModel();
			nNumPops++;
		}
	}

	if ( m_nFlags & FMESHINST_FLAG_POSTER_X ) 
	{
		// Object is to be postered around its X axis toward camera...

		fCamY_MS = FXfm_pModelView->m_MtxR.m_vPos.y;
		fCamZ_MS = FXfm_pModelView->m_MtxR.m_vPos.z;
		fCamR_MS = fmath_Sqrt( fCamY_MS*fCamY_MS + fCamZ_MS*fCamZ_MS );

		if ( fCamR_MS ) 
		{
			fCamOOR_MS = 1.0f / fCamR_MS;

			fSin = fCamOOR_MS * -fCamY_MS;
			fCos = fCamOOR_MS * fCamZ_MS;

			_XfmPosterX.m_MtxF.aa[1][1] = _XfmPosterX.m_MtxF.aa[2][2] =  fCos;
			_XfmPosterX.m_MtxR.aa[1][1] = _XfmPosterX.m_MtxR.aa[2][2] =  fCos;
			_XfmPosterX.m_MtxF.aa[2][1] = _XfmPosterX.m_MtxR.aa[1][2] = -fSin;
			_XfmPosterX.m_MtxF.aa[1][2] = _XfmPosterX.m_MtxR.aa[2][1] =  fSin;

			_XfmPosterX.PushModel();
			nNumPops++;
		}
	}

	if ( m_nFlags & FMESHINST_FLAG_POSTER_Z ) 
	{
		// Object is to be postered around its Z axis toward camera...

		fCamX_MS = FXfm_pModelView->m_MtxR.m_vPos.x;
		fCamY_MS = FXfm_pModelView->m_MtxR.m_vPos.y;
		fCamR_MS = fmath_Sqrt( fCamX_MS*fCamX_MS + fCamY_MS*fCamY_MS );

		if ( fCamR_MS ) 
		{
			fCamOOR_MS = 1.0f / fCamR_MS;

			fSin = fCamOOR_MS * fCamX_MS;
			fCos = fCamOOR_MS * fCamY_MS;

			_XfmPosterZ.m_MtxF.aa[0][0] = _XfmPosterZ.m_MtxF.aa[1][1] =  fCos;
			_XfmPosterZ.m_MtxR.aa[0][0] = _XfmPosterZ.m_MtxR.aa[1][1] =  fCos;
			_XfmPosterZ.m_MtxF.aa[0][1] = _XfmPosterZ.m_MtxR.aa[1][0] = -fSin;
			_XfmPosterZ.m_MtxF.aa[1][0] = _XfmPosterZ.m_MtxR.aa[0][1] =  fSin;

			_XfmPosterZ.PushModel();
			nNumPops++;
		}
	}

	return nNumPops;
}


//
//
//
BOOL CFMeshInst::CollideWithMeshTris( CFCollInfo *pCollInfo ) 
{
	u32 nPrevImpactCount;

	FASSERT( _bModuleInitialized );

	// If this mesh has no collision data, then return
	if ( !m_pMesh->paCollTree )
	{
		return FALSE;
	}

	nPrevImpactCount = FColl_nImpactCount;

	switch( pCollInfo->nCollTestType ) 
	{
		case FMESH_COLLTESTTYPE_SPHERE:
			/*
			if ( pCollInfo->nSphereListCount > Fang_ConfigDefs.nMesh_MaxCollSpheres ) 
			{
				DEVPRINTF( "CFMeshInst::CollideSphereListWithMeshTris(): %u spheres passed in. A max of %u is allowed.\n", pCollInfo->nSphereListCount, Fang_ConfigDefs.nMesh_MaxCollSpheres );
				pCollInfo->nSphereListCount = Fang_ConfigDefs.nMesh_MaxCollSpheres;
			}
			*/

			fmesh_Coll_TestSphere( this, pCollInfo );

			break;

		case FMESH_COLLTESTTYPE_RAY:

			fmesh_Coll_TestRay( this, pCollInfo );

			break;

		case FMESH_COLLTESTTYPE_PROJSPHERE:

			fmesh_Coll_TestProjSphere( this, pCollInfo );

			break;

		default:
			FASSERT_NOW;
	}

	return (FColl_nImpactCount > nPrevImpactCount);
}


//
//
//
static void _ArrangePlanarVertsInFan( const CFVec3 *pVerts, u8 *pVertIdx, u32 nVertCount, const CFVec3A *pPlaneNormal )
{
	FASSERT( _bModuleInitialized );
	
	u32 ii, iii;
	CFVec3A vDiff, vOrigDiff, vCross;
	f32 fSortVal[FCONVHULL_MAX_PLANE_POINTS];
	
	if ( nVertCount > FCONVHULL_MAX_PLANE_POINTS )
	{
		FASSERT_NOW;
		return;
	}
	
	vOrigDiff.v3 = pVerts[pVertIdx[1]] - pVerts[pVertIdx[0]];
	vOrigDiff.Unitize();
	vCross.Cross( vOrigDiff, *pPlaneNormal );

	fSortVal[0] = -1.570796f;
	fSortVal[1] = 1.570796f;

	for ( ii = 2; ii < nVertCount; ii++ )
	{
		vDiff.v3 = pVerts[pVertIdx[ii]] - pVerts[pVertIdx[0]];
		vDiff.Unitize();
		f32 x = vCross.Dot( vDiff );
		f32 y = vOrigDiff.Dot( vDiff );
		fSortVal[ii] = fmath_Atan( y, x );
	}

	for ( ii = 0; ii < nVertCount - 1; ii++ )
	{
		for ( iii = ii + 1; iii < nVertCount; iii++ )
		{
			if ( fSortVal[iii] > fSortVal[ii] )
			{
				continue;
			}
			
			f32 fTemp = fSortVal[ii];
			u8 nTemp = pVertIdx[ii];
			fSortVal[ii] = fSortVal[iii];
			pVertIdx[ii] = pVertIdx[iii];
			fSortVal[iii] = fTemp;
			pVertIdx[iii] = nTemp;
		}
	}
}


//
//
//
const FkDOP_Geo_t* CFMeshInst::GetkDOPGeo( u32 nBoneIdx )
{
	FASSERT( _bModuleInitialized );
	
	if ( !m_pMesh->paCollTree )
	{
		// If this mesh has no collision info, bail
		return NULL;
	}
	
	s32 i, ii, iii;
	FkDOP_Geo_t *pResult = NULL;
	FkDOP_Face_t TempFaces[26];
	u32 nkDOPType = m_pMesh->paCollTree[0].nRootkDOPType;
	
	// Find the next available geo struct
	i =  _nkDOPGeoPoolIdx;
	ii = i - 1;
	if ( ii < 0 )
	{
		ii = _KDOP_GEO_POOL_SIZE;
	}
	
	while ( i != ii )
	{
		if ( i == _KDOP_GEO_POOL_SIZE )
		{
			i = 0;
		}
		if ( !(_akDOPGeoPool[ i ].nFlags & FKDOP_GEO_FLAGS_INUSE) )
		{
			pResult = &_akDOPGeoPool[i];
			break;
		}
		
		i++;
	}

	if ( !pResult )
	{
		// Unable to find an unused Geo struct, so bail
		return NULL;
	}
	
	// Set the Geo pool index
	_nkDOPGeoPoolIdx = i + 1;
	if ( _nkDOPGeoPoolIdx == _KDOP_GEO_POOL_SIZE )
	{
		_nkDOPGeoPoolIdx = 0;
	}
	
	const FkDOP_DOPDescriptor_t *pDesc = &FkDOP_aDesc[nkDOPType];
	s32 nNorms = pDesc->nNormalCount;

	// Find the Tree that refers to the segment with the given bone index
	s32 nTreeIdx = -1;
	for ( ii = 0; ii < m_pMesh->nCollTreeCount; ii++ )
	{
		if ( m_pMesh->aSeg[m_pMesh->paCollTree[ii].nSegmentIdx].anBoneMtxIndex[0] == nBoneIdx )
		{
			nTreeIdx = ii;
		}
	}
	
	if ( nTreeIdx < 0 )
	{
		// Make sure we found a valid segment
		return NULL;
	}
	
	// Gather pointers to the relevant collision data
	FkDOP_Tree_t *pCollTree = &m_pMesh->paCollTree[nTreeIdx];
	FkDOP_Interval_t *pIntervals = pCollTree->paIntervals;
	CFVec3 *akDOPVerts = pCollTree->paRootkDOPVerts;
	if ( !pIntervals || !akDOPVerts )
	{
		return NULL;
	}

	// Clear the face array
	fang_MemSet( TempFaces, 0, sizeof( FkDOP_Face_t ) * 26 );
	
	// Go through the kDOP normals determining which verts belong to which face
	s32 nValidFaceCount = 0;
	for ( ii = 0; ii < nNorms; ii++ )
	{
		for ( iii = 0; iii < pCollTree->nRootkDOPVertCount; iii++ )
		{
			if ( pIntervals[ii].fMax - pIntervals[ii].fMin < 0.05f )
			{
				continue;
			}
			
			f32 fDot = akDOPVerts[iii].Dot( pDesc->avNormals[ii].v3 );
			f32 fDiff = fDot - pIntervals[ii].fMin;
			if ( fDiff > -0.002f && fDiff < 0.002f )
			{
				FASSERT( TempFaces[ii].nVertCount < FKDOP_MAX_DOP_FACE_VERTS );
				TempFaces[ii].anVertIdx[TempFaces[ii].nVertCount++] = (s8)iii;
				continue;
			}
			fDiff = fDot - pIntervals[ii].fMax;
			if ( fDiff > -0.002f && fDiff < 0.002f )
			{
				FASSERT( TempFaces[ii + nNorms].nVertCount < FKDOP_MAX_DOP_FACE_VERTS );
				TempFaces[ii + nNorms].anVertIdx[TempFaces[ii + nNorms].nVertCount++] = (s8)iii;
				continue;
			}
		}
		
		if ( TempFaces[ii].nVertCount > 2 )
		{
			nValidFaceCount++;
		}
		if ( TempFaces[ii + nNorms].nVertCount > 2 )
		{
			nValidFaceCount++;
		}
	}

	// Find nValidFaceCount consecutive faces in the face pool
	i = _nkDOPFacePoolIdx;
	ii = i - 1;
	iii = 0; // This will be used to count the consecutive faces
	if ( ii < 0 )
	{
		ii = _KDOP_FACE_POOL_SIZE;
	}
	
	while ( iii < nValidFaceCount && i != ii )
	{
		if ( i == _KDOP_FACE_POOL_SIZE )
		{
			iii = 0; // reset the consecutive face counter because our array cannot span past the end
			i = 0;
		}

		if ( _akDOPFacePool[i].nFlags & FKDOP_FACE_FLAGS_INUSE )
		{
			// We encountered a face that is inuse so we must reset the counter
			iii = 0;
		}
		else
		{
			iii++;
		}
		
		i++;
	}
	
	if ( i == ii )
	{
		// Couldn't find enough consecutive faces so bail.
		return NULL;
	}

	// Set the face pool index
	_nkDOPFacePoolIdx = i;
	if ( _nkDOPFacePoolIdx == _KDOP_FACE_POOL_SIZE )
	{
		_nkDOPFacePoolIdx = 0;
	}
	
	// We've got enough faces so start filling in the geometry info
	pResult->nFlags = FKDOP_GEO_FLAGS_INUSE;
	pResult->pMesh = m_pMesh;
	pResult->aFaces = &_akDOPFacePool[i - iii];
	pResult->nFaceCount = (u8)nValidFaceCount;
	pResult->aVerts = akDOPVerts;
	pResult->nVertCount = (u8)pCollTree->nRootkDOPVertCount;
	pResult->nBoneIdx = (u8)nBoneIdx;

	s32 nFaceIdx = 0;
	for ( i = 0; i < nNorms; i++ )
	{
		if ( TempFaces[i].nVertCount > 2 )
		{
			// This is the really expensive part of this function...
			// Organize the verts for this face in fan order
			_ArrangePlanarVertsInFan( akDOPVerts, TempFaces[i].anVertIdx, TempFaces[i].nVertCount, &pDesc->avNormals[i] );
			
			pResult->aFaces[nFaceIdx] = TempFaces[i];
			pResult->aFaces[nFaceIdx].nFaceNormalIdx = (u8)(i + nNorms);
			pResult->aFaces[nFaceIdx].nFlags = FKDOP_FACE_FLAGS_INUSE | FKDOP_FACE_FLAGS_INVERT_NORMAL;
			nFaceIdx++;
		}
		if ( TempFaces[i + nNorms].nVertCount > 2 )
		{
			// This is the really expensive part of this function...
			// Organize the verts for this face in fan order
			_ArrangePlanarVertsInFan( akDOPVerts, TempFaces[i + nNorms].anVertIdx, TempFaces[i + nNorms].nVertCount, &pDesc->avNormals[i] );
			
			pResult->aFaces[nFaceIdx] = TempFaces[i + nNorms];
			pResult->aFaces[nFaceIdx].nFaceNormalIdx = (u8)i;
			pResult->aFaces[nFaceIdx].nFlags = FKDOP_FACE_FLAGS_INUSE;
			nFaceIdx++;
		}
		
		FASSERT( nFaceIdx <= nValidFaceCount );
	}
	
	return pResult;
}


//
//
//
void CFMeshInst::ReleasekDOPGeo( const FkDOP_Geo_t *pGeo )
{
	FASSERT( _bModuleInitialized );
	
	((FkDOP_Geo_t *)pGeo)->nFlags = FKDOP_GEO_FLAGS_NONE;
	
	u32 i;
	for ( i = 0; i < pGeo->nFaceCount; i++ )
	{
		((FkDOP_Geo_t *)pGeo)->aFaces[i].nFlags = FKDOP_FACE_FLAGS_NONE;
	}
}


//
//
//
BOOL CFMeshInst::kDOP_TransformToWS( s32 nSegmentIdx/*=-1*/ )
{
	u16 i;
	CFMtx43A mtxBone;
	
	if ( !m_pMesh->paCollTree )
	{	
		return FALSE;
	}
	
	for ( i = 0; i < m_pMesh->nCollTreeCount; i++ )
	{
		if ( nSegmentIdx != -1 && m_pMesh->paCollTree[i].nSegmentIdx != nSegmentIdx )
		{
			continue;
		}

		if ( m_pMesh->aSeg[m_pMesh->paCollTree[i].nSegmentIdx].nBoneMtxCount != 1 )
		{
			continue;
		}

		if ( !fmesh_Coll_CalculateDOPinWS( this, i ) )
		{
			return FALSE;
		}
	}
	
	return TRUE;
}


//
// This function will draw the root kDOP for the segment using the given bone index.
// If the bone index is -1, then all kDOPs will be drawn.
//
void CFMeshInst::kDOP_Draw( CFColorRGBA *pColor/*=NULL*/, s32 nDrawBoneIdx/*=-1*/, BOOL bKeepAxisAligned/*=TRUE*/ )
{
#if !FANG_PRODUCTION_BUILD
	u32 i;
	CFMtx43A mtxBone;
	if ( !m_pMesh->paCollTree )
	{	
		return;
	}
	
	for ( i = 0; i < m_pMesh->nCollTreeCount; i++ )
	{
		if ( m_pMesh->aSeg[m_pMesh->paCollTree[i].nSegmentIdx].nBoneMtxCount != 1 )
		{
			continue;
		}
		
		u32 nBoneIdx = m_pMesh->aSeg[m_pMesh->paCollTree[i].nSegmentIdx].anBoneMtxIndex[0];
		if ( nDrawBoneIdx != -1 && nDrawBoneIdx != nBoneIdx )
		{
			continue;
		}
		
		FMesh_Coll_TransformedkDOP_t *pTransDOP = fmesh_Coll_CalculateDOPinWS( this, i );
		if ( pTransDOP )
		{
			FkDOP_DrawkDOP( pTransDOP->aIntervals, NULL, m_pMesh->paCollTree[i].nTreekDOPType );
		}
	}
#endif
}


//
//
//
BOOL CFMeshInst::ConvertMeshCollTokDOP( CFVec3A *pvCollision_WS, CFVec3A *pvNormalToOrigin_WS, s32 *pnBoneIdx, CFVec3A *pvResult_WS, s32 *pnDOPNormalIdx )
{
	FASSERT( pvCollision_WS && pvNormalToOrigin_WS && pnBoneIdx && pvResult_WS && pnDOPNormalIdx );

	u32 i, ii;
	CFMtx43A mtxConcat, mtxInvConcat;

	// Make sure we actually have kDOP collision data for this mesh
	if ( !m_pMesh->paCollTree )
	{
		return FALSE;
	}

	f32 fFurthestColl = -FMATH_MAX_FLOAT;
	

	// Loop through the segments looking for the appropriate collision
	// Port: GameCube meshes converted by port/gcmesh.cpp may carry extra segments with no collision tree
	for ( i = 0; i < m_pMesh->nSegCount && i < m_pMesh->nCollTreeCount; i++ )
	{
		if ( m_pMesh->aSeg[i].nBoneMtxCount > 1	|| !m_pMesh->paCollTree[i].paIntervals )
		{
			// Make sure the segment is not skinned and has interval info
			continue;
		}

		// Get the bone index for this segment
		u32 nBoneIdx = m_pMesh->aSeg[i].anBoneMtxIndex[0];
		
		if ( *pnBoneIdx != -1 && m_pMesh->aSeg[i].anBoneMtxIndex[0] != *pnBoneIdx )
		{
			// A bone index was specified, so we can check just the kDOP for that bone
			continue;
		}

		// Get the appropriate matrix to represent this bone in world space
		if ( m_pMesh->nBoneCount == 0 )
		{
			mtxConcat.Set( m_Xfm.m_MtxF );
			mtxInvConcat.Set( m_Xfm.m_MtxR );
		}
		else if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) ) 
		{
			FASSERT( GetBoneMtxPalette() );
			mtxConcat.Set( *GetBoneMtxPalette()[ nBoneIdx ] );
			mtxInvConcat.ReceiveAffineInverse( mtxConcat, TRUE );
		}
		else
		{
			mtxConcat.Mul( m_Xfm.m_MtxF, m_pMesh->pBoneArray[nBoneIdx].AtRestBoneToModelMtx );
			mtxInvConcat.ReceiveAffineInverse( mtxConcat, TRUE );
		}

		// Bring the point into bone space
		CFVec3A vTemp;
		mtxInvConcat.MulPoint( vTemp, *pvCollision_WS );

		u32 nkDOPType = m_pMesh->paCollTree[i].nTreekDOPType;
		u32 nNorms = FkDOP_aDesc[nkDOPType].nNormalCount;
		const CFVec3A *pkDOPNormals = FkDOP_aDesc[nkDOPType].avNormals;

		// Find the point's intervals and compare against the kDOP
		f32 fDots[26];
		FkDOP_Interval_t *pIntervals = m_pMesh->paCollTree[i].paIntervals;
		for ( ii = 0; ii < nNorms; ii++ )
		{
			fDots[ii] = vTemp.Dot( pkDOPNormals[ii] );
			if ( fDots[ii] + 0.001f < pIntervals[ii].fMin || fDots[ii] - 0.001f > pIntervals[ii].fMax )
			{
				break;
			}
		}

		if ( ii < nNorms )
		{
			// We found the point outside a set of intervals for the kDOP so no collision can exist
			continue;
		}

		CFVec3A vNorm_MS;
		mtxInvConcat.MulDir( vNorm_MS, *pvNormalToOrigin_WS );

		// Find the closest collision of the ray with this kDOP
		f32 fDist, fDot, fClosestDist = FMATH_MAX_FLOAT;
		u8 nInterval;
		u8 nOppositeSide, nOpp;
		for ( ii = 0; ii < nNorms; ii++ )
		{
			fDot = vNorm_MS.Dot( pkDOPNormals[ii] );
			if ( fDot == 0.f )
			{
				continue;
			}
			else if ( fDot < 0.f )
			{
				fDist = pIntervals[ii].fMin - fDots[ii];
				nOpp = 1;
			}
			else
			{
				fDist = pIntervals[ii].fMax - fDots[ii];
				nOpp = 0;
			}

			fDist *= fmath_Inv( fDot );
			if ( fDist <= 0.f )
			{
				continue;
			}

			if ( fDist < fClosestDist )
			{
				fClosestDist = fDist;
				nInterval = (u8)ii;
				nOppositeSide = nOpp;
			}
		}

		// Compare that against the farthest kDOP collision (We want the first kDOP
		// the ray hit on its way to collide with the mesh).
		if ( fClosestDist > fFurthestColl )
		{
			fFurthestColl = fClosestDist;
			pvResult_WS->Mul( vNorm_MS, fClosestDist ).Add( vTemp );
			mtxConcat.MulPoint( *pvResult_WS );
			(*pnDOPNormalIdx) = nInterval + (nOppositeSide * FkDOP_aDesc[nkDOPType].nNormalCount);
			(*pnBoneIdx) = nBoneIdx;
		}
	}

	if ( fFurthestColl == -FMATH_MAX_FLOAT )
	{
		// If we never found a collision, let the calling function know
		return FALSE;
	}

	// We found collision!
	return TRUE;
}


u8 CFMeshInst::m_anSaveData[ FMESH_MAX_LAYER_SAVE_SLOTS * (sizeof(CFMeshAnimTC) + sizeof(CFMeshTexFlip)) ];


const void *CFMeshInst::GetSaveData( u32 *pnSizeInBytes )
{
	FASSERT( _bModuleInitialized );

	u32 i, nByteCount;
	u32 nSaveSlotBytes = sizeof(CFMeshAnimTC) + sizeof(CFMeshTexFlip);
	u32 nMaxBytes = nSaveSlotBytes * m_pMesh->nTexLayerIDCount;
	u8 *pnData;

	if ( nMaxBytes > (nSaveSlotBytes * FMESH_MAX_LAYER_SAVE_SLOTS) )
	{
		DEVPRINTF( "CFMeshInst::GetSaveData(): Not enough room in CFMeshInst::m_anSaveData[] to hold save data for '%s'.\n", m_pMesh->szName );
		if ( pnSizeInBytes )
		{
			*pnSizeInBytes = 0;
		}
		return (void *)m_anSaveData;
	}

	nByteCount = 0;
	pnData = m_anSaveData;
	for ( i=0; m_pMesh->nTexLayerIDCount; ++i )
	{
		if ( m_aTexLayerID[i].pAnimTC )
		{
			FASSERT( (nByteCount + sizeof(CFMeshAnimTC)) <= nMaxBytes );

			fang_MemCopy( pnData, &m_aTexLayerID[i].pAnimTC, sizeof(CFMeshAnimTC) );
			pnData += sizeof(CFMeshAnimTC);
			nByteCount += sizeof(CFMeshAnimTC);
		}
		if ( m_aTexLayerID[i].pTexFlip )
		{
			FASSERT( (nByteCount + sizeof(CFMeshTexFlip)) <= nMaxBytes );

			fang_MemCopy( pnData, &m_aTexLayerID[i].pTexFlip, sizeof(CFMeshTexFlip) );
			pnData += sizeof(CFMeshTexFlip);
			nByteCount += sizeof(CFMeshTexFlip);
		}
	}

	if ( pnSizeInBytes )
	{
		*pnSizeInBytes = nByteCount;
	}

	return (void *)m_anSaveData;
}


void CFMeshInst::SetSaveData( const void *pData )
{
	FASSERT( _bModuleInitialized );

	u32 i;
	u8 *pnData;

	pnData = m_anSaveData;
	for ( i=0; m_pMesh->nTexLayerIDCount; ++i )
	{
		if ( m_aTexLayerID[i].pAnimTC )
		{
			fang_MemCopy( &m_aTexLayerID[i].pAnimTC, pnData, sizeof(CFMeshAnimTC) );
			pnData += sizeof(CFMeshAnimTC);
		}
		if ( m_aTexLayerID[i].pTexFlip )
		{
			fang_MemCopy( &m_aTexLayerID[i].pTexFlip, pnData, sizeof(CFMeshTexFlip) );
			pnData += sizeof(CFMeshTexFlip);
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	The "Normal Sphere" is a magical device.  It was created to conserve memory
//	on some platforms (initially GC).  The normal sphere consists of around 32k normals
//	compressed to 8-bit fixed point that are used to represent all of the normals 
//	in the geometry data (for lighting purposes).  The effect of this is two-fold:
//	normals are compressed to 8-bit, which affects their accuracy; and normals are
//	"rounded" off to one of the 32k existing normals in order that a u16 index can
//	be used to represent the normal.  Where more accuracy is desired (collision
//	data, for instance), the structures can create their own normals (compressed or
//	not) in order to gain more accuracy.
//

FMesh_CNorm8_t *FMesh_avCNormalSphere;
static BOOL _bNormalSphereInitialized = FALSE;

//
// This should be called from the platform specific fmesh setup if normal sphere is desired
//
void fmesh_InitNormalSphere( void )
{
	FMesh_avCNormalSphere = (FMesh_CNorm8_t *)fres_Alloc( FMESH_NORMAL_SPHERE_MAX_INDEX * sizeof( FMesh_CNorm8_t ) );
	fang_MemZero( FMesh_avCNormalSphere, FMESH_NORMAL_SPHERE_MAX_INDEX * sizeof( FMesh_CNorm8_t ) );

	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_NULL].nx = 0;
	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_NULL].ny = 0;
	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_NULL].nz = 0;
	
	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_UP].nx = 0;
	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_UP].ny = 64;
	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_UP].nz = 0;
	
	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_DOWN].nx = 0;
	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_DOWN].ny = -64;
	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_DOWN].nz = 0;

	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_RIGHT].nx = 64;
	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_RIGHT].ny = 0;
	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_RIGHT].nz = 0;

	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_LEFT].nx = -64;
	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_LEFT].ny = 0;
	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_LEFT].nz = 0;

	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_FORWARD].nx = 0;
	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_FORWARD].ny = 0;
	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_FORWARD].nz = 64;

	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_BACKWARD].nx = 0;
	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_BACKWARD].ny = 0;
	FMesh_avCNormalSphere[FMESH_NORMAL_SPHERE_NORMAL_BACKWARD].nz = -64;

	f32 fAngle;
	u32 i, iIndex = 1;
	for ( fAngle = 0; fAngle <= 90; fAngle += 1.f )
	{
		// Calculate Y
		u8 y = (u8)(cos( FMATH_DEG2RAD(fAngle) ) * FMESH_NORMAL_SPHERE_FIXED_POINT);
		f32 fSliceRadius = (f32)sin( FMATH_DEG2RAD(fAngle) );
		f32 fIncrement = 90.f / fAngle;
		for ( i = (u32)fAngle; i > 0; i-- )
		{
			u8 x = (u8)(fSliceRadius * sin( FMATH_DEG2RAD( fIncrement * (f32)i ) ) * FMESH_NORMAL_SPHERE_FIXED_POINT);
			u8 z = (u8)(fSliceRadius * cos( FMATH_DEG2RAD( fIncrement * (f32)i ) ) * FMESH_NORMAL_SPHERE_FIXED_POINT);
			
			// Upper Hemisphere
			FMesh_avCNormalSphere[iIndex].nx = x;
			FMesh_avCNormalSphere[iIndex].ny = y;
			FMesh_avCNormalSphere[iIndex].nz = z;
			
			FMesh_avCNormalSphere[iIndex + FMESH_NORMAL_SPHERE_OCT_OFFSET].nx = -x;
			FMesh_avCNormalSphere[iIndex + FMESH_NORMAL_SPHERE_OCT_OFFSET].ny = y;
			FMesh_avCNormalSphere[iIndex + FMESH_NORMAL_SPHERE_OCT_OFFSET].nz = z;

			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 2)].nx = -x;
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 2)].ny = y;
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 2)].nz = -z;
			
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 3)].nx = x;
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 3)].ny = y;
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 3)].nz = -z;
			
			// Lower Hemisphere
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 4)].nx = x;
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 4)].ny = -y;
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 4)].nz = z;
			
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 5)].nx = -x;
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 5)].ny = -y;
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 5)].nz = z;
			
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 6)].nx = -x;
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 6)].ny = -y;
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 6)].nz = -z;
			
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 7)].nx = x;
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 7)].ny = -y;
			FMesh_avCNormalSphere[iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 7)].nz = -z;

			iIndex++;
			FASSERT( iIndex + (FMESH_NORMAL_SPHERE_OCT_OFFSET * 7) < FMESH_NORMAL_SPHERE_MAX_INDEX );
		}
	}
	
	_bNormalSphereInitialized = TRUE;
}


//
//
//
u16 fmesh_GetNormalSphereIndex( f32 fx, f32 fy, f32 fz )
{
#ifdef _DEBUG
	f32 fDot = (fx * fx) + (fy * fy) + (fz * fz);
	FASSERT( fDot > 0.99f );
#endif
	if ( !_bNormalSphereInitialized )
	{
		FASSERT_NOW;
		return FMESH_NORMAL_SPHERE_NORMAL_NULL;
	}

	f32 fOOFixPoint = 1.f / (f32)FMESH_NORMAL_SPHERE_FIXED_POINT;
	if ( fy > 0.996078f )
		return FMESH_NORMAL_SPHERE_NORMAL_UP;
		
	else if ( fy < -0.996078f )
		return FMESH_NORMAL_SPHERE_NORMAL_DOWN;
		
	else if ( fx > 0.996078f )
		return FMESH_NORMAL_SPHERE_NORMAL_RIGHT;
		
	else if ( fx < -0.996078f )
		return FMESH_NORMAL_SPHERE_NORMAL_LEFT;
		
	else if ( fz > 0.996078f )
		return FMESH_NORMAL_SPHERE_NORMAL_FORWARD;
		
	else if ( fz < -0.996078f )
		return FMESH_NORMAL_SPHERE_NORMAL_BACKWARD;
		
	u32 i;
	u32 nTest, nClosest = 999999;
	s32 nClosestIndex = FMESH_NORMAL_SPHERE_NORMAL_NULL;
	u32 nIndexAdjust = 1;
	
	s8 sx = (s8)(fx * FMESH_NORMAL_SPHERE_FIXED_POINT);
	s8 sy = (s8)(fy * FMESH_NORMAL_SPHERE_FIXED_POINT);
	s8 sz = (s8)(fz * FMESH_NORMAL_SPHERE_FIXED_POINT);

	if ( sx < 0.f && sy >= 0.f && sz >= 0.f )
		nIndexAdjust = (FMESH_NORMAL_SPHERE_OCT_OFFSET);
	else if ( sx < 0.f && sy >= 0.f && sz < 0.f )
		nIndexAdjust = (FMESH_NORMAL_SPHERE_OCT_OFFSET * 2);
	else if ( sx >= 0.f && sy >= 0.f && sz < 0.f )
		nIndexAdjust = (FMESH_NORMAL_SPHERE_OCT_OFFSET * 3);
	else if ( sx >= 0.f && sy < 0.f && sz >= 0.f )
		nIndexAdjust = (FMESH_NORMAL_SPHERE_OCT_OFFSET * 4);
	else if ( sx < 0.f && sy < 0.f && sz >= 0.f )
		nIndexAdjust = (FMESH_NORMAL_SPHERE_OCT_OFFSET * 5);
	else if ( sx < 0.f && sy < 0.f && sz < 0.f )
		nIndexAdjust = (FMESH_NORMAL_SPHERE_OCT_OFFSET * 6);
	else if ( sx >= 0.f && sy < 0.f && sz < 0.f )
		nIndexAdjust = (FMESH_NORMAL_SPHERE_OCT_OFFSET * 7);

	s8 dx, dy, dz;

	// Y values increment in the lower sphere, so we need to 
	// test differently because of our range optimization
	if ( sy >= 0 )
	{
		for ( i = 1 + nIndexAdjust; i < FMESH_NORMAL_SPHERE_OCT_OFFSET + nIndexAdjust; i++ )
		{
			dy = FMesh_avCNormalSphere[i].ny - sy;

			// We are only going to test the values within a range of the
			// y value that was submitted in order to speed us along
			if ( dy > 10 )
				continue;

			if ( dy < -10 )
				break;

			dx = FMesh_avCNormalSphere[i].nx - sx;
			dz = FMesh_avCNormalSphere[i].nz - sz;

			nTest = (dx * dx) + (dy * dy) + (dz * dz );
			if ( nTest > nClosest )
				continue;

			nClosestIndex = i;
			nClosest = nTest;
		}
	}
	else
	{
		for ( i = 1 + nIndexAdjust; i < FMESH_NORMAL_SPHERE_OCT_OFFSET + nIndexAdjust; i++ )
		{
			dy = FMesh_avCNormalSphere[i].ny - sy;

			if ( dy < -10 )
				continue;

			if ( dy > 10 )
				break;

			dx = FMesh_avCNormalSphere[i].nx - sx;
//			dy = FMesh_avCNormalSphere[i].ny - sy;
			dz = FMesh_avCNormalSphere[i].nz - sz;

			nTest = (dx * dx) + (dy * dy) + (dz * dz );
			if ( nTest > nClosest )
				continue;

			nClosestIndex = i;
			nClosest = nTest;
		}
	}
	
#ifdef _DEBUG
	fDot  = (FMesh_avCNormalSphere[nClosestIndex].nx * fOOFixPoint) * fx;
	fDot += (FMesh_avCNormalSphere[nClosestIndex].ny * fOOFixPoint) * fy;
	fDot += (FMesh_avCNormalSphere[nClosestIndex].nz * fOOFixPoint) * fz;

	if ( fDot < 0.97f )
	{
		FASSERT_NOW;
	}
#endif

	return (u16)nClosestIndex;
}

