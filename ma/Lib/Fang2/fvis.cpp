//////////////////////////////////////////////////////////////////////////////////////
// fvis.cpp - Fang visibility determination system.
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
// 04/23/02 Lafleur       Created.
//////////////////////////////////////////////////////////////////////////////////////


#include "fang.h"
#include "fvis.h"

#include "fperf.h"
#include "fdraw.h"
#include "fkDOP.h"
#include "ftext.h"
#include "fclib.h"
#include "fmesh_coll.h"
#include "fworld_coll.h"
#include "fRenderSort.h"
#include "fsh.h"
#include "frenderer.h"
#include "fDataStreaming.h"

#include "fliquid.h"
#include "flight.h"

#include "fshadow.h"

#if FANG_PLATFORM_GC
	#include "fgc.h"
#endif

//////////////////////////////////////////////////////////////////////////////////////
// Local Defines
//////////////////////////////////////////////////////////////////////////////////////

#define _ENABLE_VOLUME_DRAWING		(TRUE & !FANG_PRODUCTION_BUILD)

#define _PORTAL_MIRRORS_ENABLED		FALSE
#define _MIRROR_HACK				(FALSE & _PORTAL_MIRRORS_ENABLED)
#define _DRAW_VIS_FRUSTRUMS			FALSE

// If true, we will only be allowed to traverse through a portal a set number of times
// before further traversal through that portal will be blocked.  This was an early
// attempt to prevent circular traversals but may still be needed if the current
// implementation fails to work
#define _LIMIT_PORTAL_TRAVERSAL_COUNT	FALSE		

#define _MAX_ANTIPORTALS_IN_VIEW	6
#define _MAX_MIRRORS_IN_VIEW		4
#define _MAX_DIR_LIGHTS				4
#define _AV_CLIP_BUFFER_SIZE		(FVIS_MAX_PORTAL_VERTS * 8)


//////////////////////////////////////////////////////////////////////////////////////
// Local structure and classes
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
FCLASS_ALIGN_PREFIX struct _AntiPortalOccluder
{
	u32 		nNormalCount;
	CFVec3A 	vFrustrumNormals[FVIS_MAX_PORTAL_VERTS];
	f32			fScreenArea;
	CFVec3A		vPlaneNormal_WS;
	CFVec3A		vPlanePoint_WS;
	
	FCLASS_STACKMEM_ALIGN( _AntiPortalOccluder );
		
} FCLASS_ALIGN_SUFFIX;


//////////////////////////////////////////////////////////////////////////////////////
// Global vars
//////////////////////////////////////////////////////////////////////////////////////

// Portal visibility data
FVisData_t *FVis_pVisData;
FVisCellTree_t *FVis_pCellTree;

// Pointer to the mesh that represents the current skybox
CFMeshInst *FVis_pSkyBox;

// Minimum distance an object has to be away from the camera to be rendered, used for reflections.
f32 FVis_fMinRenderDist;

u32 FVis_anVolumeTrackerKey[FWORLD_TRACKERTYPE_COUNT];

// Vars that determine what sorts of information is rendered (or not) on screen
BOOL8 FVis_bDrawVisibility;
BOOL8 FVis_bShowDrawStats;
BOOL8 FVis_bDrawCellsOnly;
BOOL8 FVis_bDontDrawObjectGeo;
BOOL8 FVis_bDontDrawPointSprites;
BOOL8 FVis_bInRenderTarget;
BOOL8 FVis_bAllowLiquid_RenderTarget;
BOOL8 FVis_bRenderShadows;

// The weight of the current directional light
f32 FVis_fCurrDirLightWeight;

// The current matrix the vis system is using for rendering
CFMtx43A FVis_mtxRenderCamera;
CFMtx43A FVis_mtxInvRenderCamera;

// Normal stack used to reflect clipping operations on the viewport
FViewport_t *FVis_pCurrentViewport;
CFVisNormalStack FVis_VPNormalStack;

// Active Volume list (visibible or close enough to visible) used for LOD'ing purposes
u8 FVis_anVolumeFlags[FVIS_MAX_VOLUME_COUNT];
u8 FVis_anPreviousVolumeFlags[FVIS_MAX_VOLUME_COUNT];

// Data that is maintained per viewport
CFMtx43A FVis_aLastRenderCamera[FVIS_MAX_BUFFERED_RENDERED_DATA_SLOTS];
FVisVisibleLight_t *FVis_paVisibleLights[FVIS_MAX_BUFFERED_RENDERED_DATA_SLOTS];
u16 FVis_anVisibleLightCount[FVIS_MAX_BUFFERED_RENDERED_DATA_SLOTS];
CFWorldMesh **FVis_papVisibleMeshes[FVIS_MAX_BUFFERED_RENDERED_DATA_SLOTS];
u16 FVis_anVisibleMeshCount[FVIS_MAX_BUFFERED_RENDERED_DATA_SLOTS];

// Vars used to keep track of VP FOV ratio
f32 FVis_fDefaultTanHalfFOVX;
f32 FVis_fLastVPFOVDistMod;

u32 FVis_nMeshCountInRegion;
FVis_ScreenMesh_t FVis_aMeshesInRegion[FVIS_MAX_SCREEN_MESH_RESULTS];
FVis_ScreenMesh_t *FVis_apSortedMeshesInRegion[FVIS_MAX_SCREEN_MESH_RESULTS];

u32 FVis_nCurLightIdx=0;
u32 FVis_nShadowBits[32];


//////////////////////////////////////////////////////////////////////////////////////
// Local Global vars
//////////////////////////////////////////////////////////////////////////////////////

static BOOL8 _bModuleInitialized;
static BOOL8 _bBlockOnStreamingData;
static BOOL8 _bToolVisData;
static BOOL8 _bEnableFog;

static u32  _nFVisCurrDataBufferSlot;

// World light info
//static FLightInit_t *_pAmbientLightInit;
static CFLight _WorldMeshInstDirLight;
static u32 _nDirLightCount;
static CFLight *_paDirLights;
static u32 _nWorldLightCount;
static CFWorldLight *_paWorldLights;

static u32 _nTotalMeshesDrawn;

// Array of volume meshes
static CFMeshInst *_paWorldMeshs;

//
#if _PORTAL_MIRRORS_ENABLED
	static CFMtx43A *_paMirrorViewMtx;
	static u32 _nMaxMirrorsInView;
	static u32 _nMirrorsInViewCount;
#endif // _PORTAL_MIRRORS_ENABLED

// Number of meshes drawn
static u32 _nDrawMeshCount;

// Keys to use to determine if meshes have already been added
static s32 _nDrawKey;
static u8 _nMaterialDrawnKey;

// Buffered camera info from prior frame
static FVisVolume_t *_pCamVolume;
static CFVec3A _vCameraPos;
static f32 _fCameraDepthInVolume;

// Vectors used for culling objects/portals from the viewport
static CFVec3 _vNearPlaneNormal;
static CFVec3 _vFarPlaneNormal;
static CFVec3 _vFarPlanePoint;
static f32 _fFarPlaneDist;

static u32 _nMaxIntersects;

// Buffer used for clipping portal verts to the viewport
static CFVec3A _avClipBuffer[2][_AV_CLIP_BUFFER_SIZE];


// Buffer for storing occluder information
static _AntiPortalOccluder _Occluders[ _MAX_ANTIPORTALS_IN_VIEW ];
static u32 _ActiveOccluderCount;

#if FANG_DEBUG_BUILD && _DRAW_VIS_FRUSTRUMS
// Used for drawing visibility frustums
static u32 _nPortalConsidered;
#endif


//////////////////////////////////////////////////////////////////////////////////////
// Temp Function for getting directional light for specular
//////////////////////////////////////////////////////////////////////////////////////
CFLight *fvis_GetCurrentDirectional()
{
	return _paDirLights;
}

//////////////////////////////////////////////////////////////////////////////////////
// Portal visitation bit packing 
//////////////////////////////////////////////////////////////////////////////////////

#if _LIMIT_PORTAL_TRAVERSAL_COUNT
	static u8 *_panPortalVisitCount = NULL;

	FINLINE void _ResetPortalBits( void )
	{
		FASSERT( _panPortalVisitCount );
		fang_MemZero( _panPortalVisitCount, (FVis_pVisData->nPortalCount + 3) >> 2 );
	}

	FINLINE u32 _GetTimesVisitedPortal( u16 nPortalIdx )
	{
		FASSERT( _panPortalVisitCount );
		FASSERT( nPortalIdx < FVis_pVisData->nPortalCount );
		
		u32 nIndex = nPortalIdx >> 2;
		return ( (_panPortalVisitCount[nIndex] >> ((nPortalIdx & 0x03) << 1) ) & 0x03);

	}

	FINLINE void _IncrementTimesVisitedPortal( u16 nPortalIdx )
	{
		FASSERT( _panPortalVisitCount );
		FASSERT( nPortalIdx < FVis_pVisData->nPortalCount );
		
		u32 nIndex = nPortalIdx >> 2;
		u32 nNewValue = (_GetTimesVisitedPortal( nPortalIdx ) + 1) & 0x03;
		_panPortalVisitCount[nIndex] |= (nNewValue << ((nPortalIdx & 0x03) << 1));
	}
#endif


//////////////////////////////////////////////////////////////////////////////////////
// Static function prototypes
//////////////////////////////////////////////////////////////////////////////////////

// Display list creation functions
static BOOL _DL_Create( const CFMtx43A *pCameraMtx, BOOL bDrawEntireWorld );
static void _DL_WalkVisibility( FVisVolume_t *pVolume, u32 nPortalViewedFrom );
static BOOL _DL_AddVolume( FVisVolume_t *pVolume );
//static BOOL _DL_GenerateVolumeLights_Callback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
static void _DL_AddMesh( CFWorldTracker *pTracker );
static void _DL_AddPointSprite( CFWorldTracker *pTracker );
static BOOL _ClipViewportToPortal( const CFVec3 *pPortalVerts, u32 nVolumePortalIdx );
static BOOL _ClipVertsToPlane( u32 *pnBufferIdx, u32 *pnVertCount, const CFVec3A *pPlaneNormal );

//static void _DrawLiquidVolumes();

static void _AL_AddVolume( FVisVolume_t *pVolume, u32 nPortalAddedFrom, s32 pnStepsLeft );

static void _ResDestroyWorldCallback( void *pBase );

#if _ENABLE_VOLUME_DRAWING
static void _DrawVolumePlanes( void );
#endif
#if FANG_DEBUG_BUILD && _DRAW_VIS_FRUSTRUMS
static void _DrawViewportExtents( CFVec3A *pCamPos, CFVec3A *paVerts, u32 nPointCount );
#endif


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL fvis_ModuleStartup( void ) 
{
	u32 i;
	
	// Initialize local globals
	FVis_pVisData = NULL;
	FVis_pCellTree = NULL;
	_bEnableFog = FALSE;
	
	_bToolVisData = FALSE;

	FVis_pSkyBox = NULL;

	_nMaterialDrawnKey = 0;

	FVis_fMinRenderDist = 0.0f;
	
	FVis_bDrawVisibility = FALSE;
	FVis_bShowDrawStats = FALSE;
	FVis_bDrawCellsOnly = FALSE;
	FVis_bDontDrawObjectGeo = FALSE;
	FVis_bDontDrawPointSprites = FALSE;

	_bBlockOnStreamingData = FALSE;

	_nFVisCurrDataBufferSlot = 0;

	FVis_fCurrDirLightWeight = 0.f;

	_pCamVolume = NULL;
	_vCameraPos.Set( FMATH_MAX_FLOAT, FMATH_MAX_FLOAT, FMATH_MAX_FLOAT );

	FVis_bInRenderTarget = FALSE;
	FVis_bAllowLiquid_RenderTarget = FALSE;
	FVis_bRenderShadows = TRUE;
	FVis_nCurLightIdx = 0;

	for ( i = 0; i < FVIS_MAX_BUFFERED_RENDERED_DATA_SLOTS; i++ )
	{
		FVis_paVisibleLights[i] = (FVisVisibleLight_t *)fres_AllocAndZero( FVIS_MAX_VISIBLE_LIGHTS_LIST_COUNT * sizeof(FVisVisibleLight_t) );
		if ( FVis_paVisibleLights == NULL ) 
		{
			DEVPRINTF( "fvis_ModuleStartup(): Insufficient memory to allocate %u bytes for visible lights Buffer.\n", FVIS_MAX_VISIBLE_LIGHTS_LIST_COUNT * sizeof(FVisVisibleLight_t) );
			return FALSE;
		}
		FVis_anVisibleLightCount[i] = 0;
	}

	for ( i = 0; i < FVIS_MAX_BUFFERED_RENDERED_DATA_SLOTS; i++ )
	{
		FVis_papVisibleMeshes[i] = (CFWorldMesh **)fres_AllocAndZero( FVIS_MAX_VISIBLE_MESH_LIST_COUNT * sizeof(CFWorldTracker *) );
		if ( FVis_papVisibleMeshes == NULL ) 
		{
			DEVPRINTF( "fvis_ModuleStartup(): Insufficient memory to allocate %u bytes for visible trackers Buffer.\n", FVIS_MAX_VISIBLE_MESH_LIST_COUNT * sizeof(CFWorldMesh *) );
			return FALSE;
		}
		FVis_anVisibleMeshCount[i] = 0;
	}

	_nDirLightCount = 0;
	_paDirLights = fnew CFLight[_MAX_DIR_LIGHTS];
	if ( !_paDirLights )
	{
		DEVPRINTF( "fvis_ModuleStartup(): Insufficient memory to allocate %u bytes for world lights.\n", _MAX_DIR_LIGHTS * sizeof(CFLight) );
		return FALSE;
	}

	_nWorldLightCount = 0;
	_paWorldLights = NULL;

#if _PORTAL_MIRRORS_ENABLED
	_nMaxMirrorsInView = _MAX_MIRRORS_IN_VIEW;
	_paMirrorViewMtx = fnew CFMtx43A[ _nMaxMirrorsInView ];
	if ( !_paMirrorViewMtx )
	{
		DEVPRINTF( "fvis_ModuleStartup(): Insufficient memory to allocate %u bytes for Mirror matrix Buffer.\n", _nMaxMirrorsInView * 2 * sizeof(CFMtx43A) );
		_nMaxMirrorsInView = 0;
	}
	_nMirrorsInViewCount = 0;
#endif // _PORTAL_MIRRORS_ENABLED

	// Determine default field of view
	f32 fSin, fCos;
	fmath_SinCos( FMATH_QUARTER_PI, &fSin, &fCos );
	FVis_fDefaultTanHalfFOVX = fSin / fCos;
	FVis_fDefaultTanHalfFOVX = 0.64940792f;

	_bModuleInitialized = TRUE;
	return _bModuleInitialized;
}


//
//
//
void fvis_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	if ( _paDirLights )
	{
		fdelete_array( _paDirLights );
		_paDirLights = NULL;
	}

//	if ( _paWorldLights )
//	{
//		fdelete_array( _paWorldLights );
//		_paWorldLights = NULL;
//	}

#if _PORTAL_MIRRORS_ENABLED
	if ( _paMirrorViewMtx )
	{
		fdelete_array( _paMirrorViewMtx );
		_paMirrorViewMtx = NULL;
	}
#endif // _PORTAL_MIRRORS_ENABLED

	_bModuleInitialized = FALSE;
}


//
// Set nMaxIntersects to 0 to use the value from Fang_ConfigDefs.nWorld_MaxIntersects.
//
BOOL fvis_CreateVisData( FResHandle_t hRes, FVisData_t *pVisData, u32 nMaxIntersects ) 
{
	FResFrame_t ResFrame;
	CFWorldIntersect *pIntersectPool;
	u32 i, ii;

	FASSERT( _bModuleInitialized );

	ResFrame = fres_GetFrame();

	// Clear the current sky box pointer (It would not be valid, anyway, at this point)
	FVis_pSkyBox = NULL;
	
	fres_SetBaseAndCallback( hRes, pVisData, _ResDestroyWorldCallback );

	_bBlockOnStreamingData = TRUE;

	// Clear the rendered objects
	for ( i = 0; i < FVIS_MAX_BUFFERED_RENDERED_DATA_SLOTS; i++ )
	{
		FVis_anVisibleLightCount[i] = 0;
		FVis_anVisibleMeshCount[i] = 0;
	}

	// Initialize the visibility data
	if ( !fvis_FixupPortalData( pVisData ) )
	{
		goto _ExitCreateWithError;
	}
	
	// Set the active VisData
	_bToolVisData = FALSE;
	FWorld_pWorld = FVis_pVisData = pVisData;
	FVis_pCellTree = &pVisData->CellTree;

	// Set up intersect lists
	for ( i = 0; i < FVis_pVisData->nVolumeCount; i++ )
	{
		FVisVolume_t *pVolume = &FVis_pVisData->paVolumes[i];
		if ( !pVolume )
		{
			continue;
		}
		
		for ( ii = 0; ii < FWORLD_TRACKERTYPE_COUNT; ii++ )
		{
			flinklist_InitRoot( &pVolume->aTrackerIntersects[ii], CFWorldIntersect::GetOffsetOfTrackerInNodeLink() );
		}
	}	

	CFWorldKey::InitSystem();

	// Ensure a valid intersect maximum count
	if ( nMaxIntersects == 0 ) 
	{
		if( Fang_ConfigDefs.nWorld_MaxIntersects > 0 ) 
		{
			_nMaxIntersects = Fang_ConfigDefs.nWorld_MaxIntersects;
		} 
		else 
		{
			DEVPRINTF( "fvis_Create(): Fang_ConfigDefs.nWorld_MaxIntersects must be greater than 0. Overriding.\n" );
			_nMaxIntersects = 1000;
		}
	} 
	else 
	{
		_nMaxIntersects = nMaxIntersects;
	}
	
	// Create our intersect pool...
	pIntersectPool = fnew CFWorldIntersect[_nMaxIntersects];
	if ( pIntersectPool == NULL ) 
	{
		DEVPRINTF( "fvis_Create(): Could not allocate %u bytes for CFWorldIntersect pool.\n", sizeof(CFWorldIntersect) * _nMaxIntersects );
		goto _ExitCreateWithError;
	}
	CFWorldIntersect::InitPool( pIntersectPool, _nMaxIntersects );

	CFWorldKey::ResetAllKeys();
	CFWorldMesh::ResetDrawnKey();
	CFWorldPSGroup::ResetDrawnKey();

	_pCamVolume = NULL;
	_vCameraPos.Set( FMATH_MAX_FLOAT, FMATH_MAX_FLOAT, FMATH_MAX_FLOAT );

	_bEnableFog = TRUE;

	_WorldMeshInstDirLight.InitDirLight( 0.0f, 0.0f, 1.0f );

	// Initialize the light list
	_nDirLightCount = 0;
	_nWorldLightCount = 0;
#if !FANG_PRODUCTION_BUILD	
	if ( FVis_pVisData->nLightCount > FVIS_MAX_LIGHTS )
	{
		DEVPRINTF( "fvis_Create() - WARNING! - This world has %d lights which is more than the supported limit of %d.\n", FVis_pVisData->nLightCount, FVIS_MAX_LIGHTS );
	}
#endif	

	_paWorldLights = fnew CFWorldLight[FVis_pVisData->nLightCount];
	if ( !_paWorldLights )
	{
		DEVPRINTF( "fvis_Create(): Insufficient memory to allocate %u bytes for world lights.\n", FVis_pVisData->nLightCount * sizeof(CFWorldLight) );
		goto _ExitCreateWithError;
	}
	
	FVis_nCurLightIdx = 0;

	for ( i = 0; i < FVis_pVisData->nLightCount; i++ )
	{
		if ( FVis_pVisData->paLights[i].nType > FLIGHT_TYPE_COUNT ) 
		{
			DEVPRINTF( "fvis_Create(): Invalid light type attached to world: %u (light skipped).\n", FVis_pVisData->paLights[i].nType );
			continue;
		}

		if ( FVis_pVisData->paLights[i].nType == FLIGHT_TYPE_DIR ) 
		{
			if ( _nDirLightCount < _MAX_DIR_LIGHTS )
			{
				_paDirLights[_nDirLightCount].Init( &FVis_pVisData->paLights[i] );
				_nDirLightCount++;
			}
			continue;
		}

		if ( _nWorldLightCount < FVIS_MAX_LIGHTS )
		{
			_paWorldLights[_nWorldLightCount].Init( &FVis_pVisData->paLights[i] );
			_nWorldLightCount++;
		}
	}

#if _LIMIT_PORTAL_TRAVERSAL_COUNT
	// Allocate some space to hold portal visibility counters	
	_panPortalVisitCount = (u8 *)fres_Alloc( (FVis_pVisData->nPortalCount + 3) >> 2 );
#endif
	
	return TRUE;

	// Failure...
_ExitCreateWithError:
	fres_ReleaseFrame( ResFrame );
	FWorld_pWorld = NULL;
	return FALSE;
}


//
//
//
static void _ResDestroyWorldCallback( void *pBase ) 
{
	pBase;

	u32 i;
	// Clear the rendered objects
	for ( i = 0; i < FVIS_MAX_BUFFERED_RENDERED_DATA_SLOTS; i++ )
	{
		FVis_anVisibleLightCount[i] = 0;
		FVis_anVisibleMeshCount[i] = 0;
	}

	fdelete_array( _paWorldLights );
	_paWorldLights = NULL;
	_nWorldLightCount = 0;

	fworld_AnnouncePreDestroy();
	CFWorldTracker::DeleteAllFlagedTrackers();
	fdelete_array( _paWorldMeshs );
	_paWorldMeshs = NULL;
	FWorld_pWorld = NULL;
	FVis_pSkyBox = NULL;
	fworld_AnnouncePostDestroy();
}


//
//
//
BOOL fvis_InitWorldStreamingData( void *pStreamingData, u32 nSize ) 
{
#if FANG_PLATFORM_GC
	if ( FDS_StreamMgr.StoreRawData( pStreamingData, nSize, FDS_LOCATION_ARAM ) )
	{
		return TRUE;
	}
#endif // FANG_PLATFORM_GC
	
	return FALSE;
}


//
//
//
BOOL fvis_ResolveLoadedVolumeMeshes( FVisGeoHeader_t *pGeoHeader ) 
{
	FASSERT( pGeoHeader );

	if ( !FVis_pVisData )
	{
		FASSERT_NOW;
		return FALSE;
	}

	u32 i;
	CFMtx43A mtxWorldGeo;

	FASSERT( _bModuleInitialized );

	// Initialize our meshes
	mtxWorldGeo.Identity();
	_paWorldMeshs = fnew CFMeshInst[pGeoHeader->nMeshCount];
	for ( i = 0; i < pGeoHeader->nMeshCount; i++ )
	{
		FASSERT( pGeoHeader->papMeshArray[i] );
		
		_paWorldMeshs[i].Init( pGeoHeader->papMeshArray[i] );
		_paWorldMeshs[i].m_nFlags |= FMESHINST_FLAG_WORLDSPACE | FMESHINST_FLAG_WORLD_GEO;
		_paWorldMeshs[i].m_Xfm.BuildFromMtx( mtxWorldGeo, FALSE );
		_paWorldMeshs[i].m_FinalAmbientRGB.Black();
	}
	
	// Resolve the visibility data mesh indices
	for ( i = 0; i < FVis_pVisData->nVolumeCount; i++ )
	{
		FVisVolume_t *pVolume = &FVis_pVisData->paVolumes[i];
		if ( !pVolume )
		{
			continue;
		}
		
		if ( pVolume->pWorldGeo == FVIS_INVALID_GEO_ID )
		{
			pVolume->pWorldGeo = NULL;
		}
		else
		{
			pVolume->pWorldGeo = &_paWorldMeshs[(u32)pVolume->pWorldGeo];
			pVolume->pWorldGeo->m_pMesh->nFlags |= FMESH_FLAGS_VOLUME_MESH;
		}
	}	

	return TRUE;
}


//
//
//
CFWorldLight* fvis_GetWorldLightByLightID( u32 nLightID )
{
	u32 i;
	for ( i = 0; i < _nWorldLightCount; i++ )
	{
		if ( _paWorldLights[i].m_Light.m_nLightID == nLightID )
		{
			return &_paWorldLights[i];
		}
	}

	return NULL;
}


//
//
//
CFWorldLight* fvis_GetWorldLightByMotifIndex( u32 nMotifIdx )
{
	u32 i;
	for ( i = 0; i < _nWorldLightCount; i++ )
	{
		if ( _paWorldLights[i].m_Light.m_Motif.nMotifIndex == nMotifIdx )
		{
			return &_paWorldLights[i];
		}
	}

	return NULL;
}


//
// This function should only ever be called externally by the tools
//
void fvis_SetActiveVisData( FVisData_t *pData ) 
{
	FASSERT( !FVis_pVisData || _bToolVisData );

	// Set local pointers
	if ( pData )
	{
		_bToolVisData = TRUE;
		FWorld_pWorld = FVis_pVisData = pData;
		FVis_pCellTree = &pData->CellTree;
	}
	else
	{
		_bToolVisData = FALSE;
		FWorld_pWorld = FVis_pVisData = NULL;
		FVis_pCellTree = NULL;
	}
}


#if FANG_DEBUG_BUILD
//
//
//
BOOL fvis_VerifyTrackerCounts( void )
{
	u32 i, ii, nCounter;

	for ( ii = 0; ii < FVis_pVisData->nVolumeCount; ii++ )
	{
		FVisVolume_t *pVol = &FVis_pVisData->paVolumes[ii];
		CFWorldIntersect *pIntersect = NULL;
		for ( i = 0; i < FWORLD_TRACKERTYPE_COUNT; i++ )
		{
			nCounter = 0;

			while( (pIntersect = (CFWorldIntersect *)flinklist_GetNext( &pVol->aTrackerIntersects[i], pIntersect )) ) 
			{
				nCounter++;
			}

			if ( nCounter != pVol->anTotalTrackerCount[i] )
			{
				FASSERT_NOW;
				return FALSE;
			}
		}
	}

	return TRUE;
}
#endif

//to put a value in the variable:
//nArrIdx = pLight->m_nIndex/10
//nBit = (pLight->m_nIndex - nArrIdx*10)*3
//nBits = FVis_nShadowBits[nArrIdx];
// -- SET VALUE --
//nBits &= ~( 1<<(nBit) | 1<<(nBit+1) | 1<<(nBit+2) )
//nBits |=  (nValue<<nBit)
// -- GET VALUE --
//nValue = (nBits>>nBit)&7
//
void fvis_SetNumShadows(u32 nLightIdx, u8 nValue)
{
	u32 nArrIdx = nLightIdx>>3;
	u32 nBit = (nLightIdx - (nArrIdx<<3))<<2;
	
	FASSERT(nArrIdx < 32);
	FASSERT(nBit < 30);
	
	FVis_nShadowBits[nArrIdx] &= ~( (1<<(nBit)) | (1<<(nBit+1)) | (1<<(nBit+2)) | (1<<(nBit+3)) );
	FVis_nShadowBits[nArrIdx] |= ( (nValue&0x0F)<<nBit );
}


//
//
//
u8 fvis_GetNumShadows( u32 nLightIdx )
{
	u32 nArrIdx = nLightIdx>>3;
	u32 nBit = (nLightIdx - (nArrIdx<<3))<<2;

	FASSERT(nArrIdx < 32);
	FASSERT(nBit < 30);
	
	return ( (FVis_nShadowBits[nArrIdx]>>nBit)&0x0F );
}


//
//
//
void fvis_ClearNumShadows( void )
{
	fang_MemSet(FVis_nShadowBits, 0, 128);
}

void fvis_SetupShadows( void )
{
	u32 i;
	u32 nDataIndex=0;
	//Shadow Setup
	if (FShadow_bMultiplayer && !FVis_bInRenderTarget && FVis_bRenderShadows)
	{
		fshadow_ClearLightList();

		if (_paDirLights && _nDirLightCount > 0)
		{
			if (_paDirLights[0].m_nFlags&FLIGHT_FLAG_CAST_SHADOWS)
			{
				fshadow_SubmitLight(&_paDirLights[0], NULL);
			}
		}

		fshadow_EndSubmit();
	}
	else if (!FVis_bInRenderTarget && FVis_bRenderShadows)
	{
		fshadow_ClearLightList();
		
#if 1
		if (_paDirLights && _nDirLightCount > 0)
		{
			if (_paDirLights[0].m_nFlags&FLIGHT_FLAG_CAST_SHADOWS)
			{
				fshadow_SubmitLight(&_paDirLights[0], NULL);
			}
		}
		
		for ( i = 0; i < FVis_anVisibleLightCount[nDataIndex]; i++)
		{
			if ( (FVis_paVisibleLights[nDataIndex][i].pLight->m_Light.m_nType == FLIGHT_TYPE_SPOT || FVis_paVisibleLights[nDataIndex][i].pLight->m_Light.m_nType == FLIGHT_TYPE_OMNI) && FVis_paVisibleLights[nDataIndex][i].pLight->m_Light.m_nFlags&FLIGHT_FLAG_CAST_SHADOWS )
			{
				fshadow_SubmitLight(&FVis_paVisibleLights[nDataIndex][i].pLight->m_Light, NULL);
			}
		}
		
		fshadow_EndSubmit();
#endif
		
	}
}

//
//
//
static void _RenderLightFX( const CFMtx43A *pCamera, u32 nDataIndex )
{
	//Light Coronas
	static u32 __nPhase = 0;
	
	frenderer_Push( FRENDERER_DRAW, NULL );

	int i;
    CFWorldLight *wl;

	if (FLight_bShowLightRange)
	{
		for ( i = 0; i < FVis_anVisibleLightCount[nDataIndex]; i++)
		{
			wl = FVis_paVisibleLights[nDataIndex][i].pLight;

			if (wl)		
			{
				wl->m_Light.RenderRange();
			}
		}
	}

	for ( i = 0; i < FVis_anVisibleLightCount[nDataIndex]; i++)
	{
		wl = FVis_paVisibleLights[nDataIndex][i].pLight;
		if (wl)
		{
			if ( FVis_paVisibleLights[nDataIndex][i].bOriginVisible )
			{
				wl->m_Light.RenderCorona(pCamera, FVis_paVisibleLights[nDataIndex][i].fDistFromCameraSq, __nPhase);
			}
		}
	}

	frenderer_Pop();
	
	__nPhase = (__nPhase+1)%8;
/*	
	//Shadow Setup
	if (!FVis_bInRenderTarget && FVis_bRenderShadows)
	{
		fshadow_ClearLightList();
		
		//fshadow_DrawTest();

		frenderer_Pop();
		
#if 1
		if (_paDirLights)
		{
			//test
			//_paDirLights[0].m_nFlags |= FLIGHT_FLAG_CAST_SHADOWS;
			//
			if (_paDirLights[0].m_nFlags&FLIGHT_FLAG_CAST_SHADOWS)
			{
				fshadow_SubmitLight(&_paDirLights[0], NULL);
			}
		}
		
		for ( i = 0; i < FVis_anVisibleLightCount[nDataIndex]; i++)
		{
			//test
			//FVis_paVisibleLights[nDataIndex][i].pLight->m_Light.m_nFlags |= FLIGHT_FLAG_CAST_SHADOWS;
			//
			if ( (FVis_paVisibleLights[nDataIndex][i].pLight->m_Light.m_nType == FLIGHT_TYPE_SPOT || FVis_paVisibleLights[nDataIndex][i].pLight->m_Light.m_nType == FLIGHT_TYPE_OMNI) && FVis_paVisibleLights[nDataIndex][i].pLight->m_Light.m_nFlags&FLIGHT_FLAG_CAST_SHADOWS )
			{
				fshadow_SubmitLight(&FVis_paVisibleLights[nDataIndex][i].pLight->m_Light, NULL);
			}
		}
		
		fshadow_EndSubmit();
#endif
		
	}
	*/
	//
}


//
//
//
void fvis_GetMeshPosInScreenRegion( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, CFWorldMesh *pMesh, 
							   const CFVec2 *pScreenPoint, f32 fRadius, f32 fTrackerSizeAdjust, FVis_ScreenMesh_t *pResult )
{
	FASSERT( pViewport && pCamMtx && pMesh && pScreenPoint && pResult );

	f32 fDist, fDistSq;
	CFVec2 vPos, vDiff;
	CFSphere spResult, spTracker( pMesh->GetBoundingSphere() );
	spTracker.m_fRadius *= fTrackerSizeAdjust;

	// Convert y to be the same scale as x
	vPos.x = pScreenPoint->x;
	vPos.y = pScreenPoint->y * pViewport->fAspectHOW;

	fviewport_ComputeScreenPointAndSize_WS( pViewport, pCamMtx, &spTracker, &spResult );

	// Convert y to be the same scale as x
	spResult.m_Pos.y *= pViewport->fAspectHOW;

	// Get a vector from the reticle to the tracker
	vDiff.x = (vPos.x - spResult.m_Pos.x);
	vDiff.y = (vPos.y - spResult.m_Pos.y);
	fDistSq = (vDiff.x * vDiff.x) + (vDiff.y * vDiff.y);
	if ( fDistSq > 0.0001f )
	{
		fDist = fmath_Sqrt( fDistSq );
	}
	else
	{
		fDist = 0.f;
	}
//	if ( fDist - spResult.m_fRadius < fRadius )
	{
		// This mesh is within the screen region, so record it
		pResult->pWorldMesh = pMesh;
		pResult->spScreen.m_Pos.x = spResult.m_Pos.x;
		pResult->spScreen.m_Pos.y = spResult.m_Pos.y;
		pResult->spScreen.m_Pos.z = spResult.m_Pos.z - pMesh->GetBoundingSphere().m_fRadius;
		pResult->spScreen.m_fRadius = spResult.m_fRadius;
		pResult->fDistanceFromRegion = fDist - spResult.m_fRadius;
		pResult->vNonUnitToRegion.Set( vDiff.x, vDiff.y, 0.f );
	}
}


//
//
//
u32 fvis_GetMeshesInScreenRegion( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, u32 nDataIndex,  
								 const CFVec2 *pScreenPoint, f32 fRadius, f32 fMaxDistance, f32 fTrackerSizeAdjust,
								 u64 nUserBitMask, BOOL bUseSkipList )
{
	FASSERT( pViewport && pCamMtx && pScreenPoint );
	FASSERT( nDataIndex < FVIS_MAX_BUFFERED_RENDERED_DATA_SLOTS );

	u32 i, uIndex;
	f32 fDist, fDistSq;
	f32 fRadiusSq = fRadius * fRadius;
	CFVec2 vPos, vDiff;

	// Convert y to be the same scale as x
	vPos.x = pScreenPoint->x;
	vPos.y = pScreenPoint->y * pViewport->fAspectHOW;

	FVis_nMeshCountInRegion = 0;
	CFSphere spResult;

	if ( bUseSkipList ) 
	{
		for ( i = 0; i < FVis_anVisibleMeshCount[nDataIndex]; i++ )
		{
			// Is this a tracker type the client is interested in?
			if ( !(FVis_papVisibleMeshes[nDataIndex][i]->GetUserTypeBits() & nUserBitMask) )
			{
				continue;
			}

			// Make sure the tracker is not in the skip list
			for ( uIndex = 0; uIndex < FWorld_nTrackerSkipListCount; uIndex++ )
			{
				if ( FVis_papVisibleMeshes[nDataIndex][i] == FWorld_apTrackerSkipList[uIndex] )
				{
					// Tracker is in the skip list, can't collide with this guy
					break;
				}
			}

			if ( uIndex != FWorld_nTrackerSkipListCount )
			{
				continue;
			}

			CFSphere spTracker = FVis_papVisibleMeshes[nDataIndex][i]->GetBoundingSphere();
			spTracker.m_fRadius *= fTrackerSizeAdjust;

			fviewport_ComputeScreenPointAndSize_WS( pViewport, pCamMtx, &spTracker, &spResult );

			if ( spResult.m_Pos.z - spTracker.m_fRadius > fMaxDistance )
			{
				continue;
			}

			// Convert y to be the same scale as x
			spResult.m_Pos.y *= pViewport->fAspectHOW;

			// Get a vector from the reticle to the tracker
			vDiff.x = (vPos.x - spResult.m_Pos.x);
			vDiff.y = (vPos.y - spResult.m_Pos.y);
			fDistSq = (vDiff.x * vDiff.x) + (vDiff.y * vDiff.y);
			if ( fDistSq > 0.0001f )
			{
				fDist = fmath_Sqrt( fDistSq );
			}
			else
			{
				fDist = 0.f;
			}
			if ( fDist - spResult.m_fRadius < fRadius )
			{
				// This mesh is within the screen region, so record it
				FVis_aMeshesInRegion[FVis_nMeshCountInRegion].pWorldMesh = FVis_papVisibleMeshes[nDataIndex][i];
				FVis_aMeshesInRegion[FVis_nMeshCountInRegion].spScreen.m_Pos.x = spResult.m_Pos.x;
				FVis_aMeshesInRegion[FVis_nMeshCountInRegion].spScreen.m_Pos.y = spResult.m_Pos.y;
				FVis_aMeshesInRegion[FVis_nMeshCountInRegion].spScreen.m_Pos.z = spResult.m_Pos.z - FVis_papVisibleMeshes[nDataIndex][i]->GetBoundingSphere().m_fRadius;
				FVis_aMeshesInRegion[FVis_nMeshCountInRegion].spScreen.m_fRadius = spResult.m_fRadius;
				FVis_aMeshesInRegion[FVis_nMeshCountInRegion].fDistanceFromRegion = fDist - spResult.m_fRadius;
				FVis_aMeshesInRegion[FVis_nMeshCountInRegion].vNonUnitToRegion.Set( vDiff.x, vDiff.y, 0.f );

				FVis_nMeshCountInRegion++;
				if ( FVis_nMeshCountInRegion == FVIS_MAX_SCREEN_MESH_RESULTS )
				{
					// We've reached the end of our buffer
					break;
				}
			}
		}
	}
	else
	{
		FASSERT_NOW;
/*
		for ( i = 0; i < FVis_anVisibleMeshCount[nDataIndex]; i++ )
		{
			// Is this a tracker type the client is interested in?
			if ( !(FVis_papVisibleMeshes[nDataIndex][i]->GetUserTypeBits() & nUserBitMask) )
			{
				continue;
			}

			fviewport_ComputeScreenPointAndSize_WS( pViewport, pCamMtx, &FVis_papVisibleMeshes[nDataIndex][i]->GetBoundingSphere(), &spResult );
			fDistSq =  (spResult.m_Pos.x - pScreenPoint->x) * (spResult.m_Pos.x - pScreenPoint->x);
			fDistSq += (spResult.m_Pos.y - pScreenPoint->y) * (spResult.m_Pos.y - pScreenPoint->y);
			if ( fDistSq < fRadiusSq )
			{
				FVis_papScreenMeshes[FVis_nScreenMeshCount++] = FVis_papVisibleMeshes[nDataIndex][i];
				if ( FVis_nScreenMeshCount == FVIS_MAX_SCREEN_MESH_RESULTS )
				{
					break;
				}
			}
		}
*/
	}

	return FVis_nMeshCountInRegion;
}

//
//
//
static int _SortCallback_Ascend( const void *pElement1, const void *pElement2 ) 
{
	const FVis_ScreenMesh_t *pMesh1, *pMesh2;

	pMesh1 = *(const FVis_ScreenMesh_t **)pElement1;
	pMesh2 = *(const FVis_ScreenMesh_t **)pElement2;

	if ( pMesh1->spScreen.m_Pos.z < pMesh2->spScreen.m_Pos.z ) 
	{
		return -1;
	} 
	else if ( pMesh1->spScreen.m_Pos.z > pMesh2->spScreen.m_Pos.z ) 
	{
		return 1;
	} 
	else 
	{
		return 0;
	}
}


//
//
//
static int _SortCallback_Descend( const void *pElement1, const void *pElement2 ) 
{
	const FVis_ScreenMesh_t *pMesh1, *pMesh2;

	pMesh1 = *(const FVis_ScreenMesh_t **)pElement1;
	pMesh2 = *(const FVis_ScreenMesh_t **)pElement2;

	if ( pMesh1->spScreen.m_Pos.z < pMesh2->spScreen.m_Pos.z ) 
	{
		return 1;
	} 
	else if ( pMesh1->spScreen.m_Pos.z > pMesh2->spScreen.m_Pos.z ) 
	{
		return -1;
	} 
	else 
	{
		return 0;
	}
}


//
//  If bAscend is FALSE, the list will be sorted from furthest to closest
//
void fvis_SortRegionMeshResults( BOOL bAscend/*=TRUE*/ )
{
	u32 i;

	if ( FVis_nMeshCountInRegion ) 
	{
		for ( i=0; i<FVis_nMeshCountInRegion; i++ ) 
		{
			FVis_apSortedMeshesInRegion[i] = &FVis_aMeshesInRegion[i];
		}

		if ( bAscend )
		{
			fclib_QSort( FVis_apSortedMeshesInRegion, FVis_nMeshCountInRegion, sizeof(FVis_ScreenMesh_t *), _SortCallback_Ascend );
		}
		else
		{
			fclib_QSort( FVis_apSortedMeshesInRegion, FVis_nMeshCountInRegion, sizeof(FVis_ScreenMesh_t *), _SortCallback_Descend );
		}
	}
}	


//
//
//
void fvis_FrameBegin( void )
{
	// Clear the Active Volume Flags for this player
	if ( FVis_pVisData ) 
	{ 
		// save flags before erasing
		fang_MemCopy( FVis_anPreviousVolumeFlags, FVis_anVolumeFlags, FVis_pVisData->nVolumeCount );

		fang_MemSet( FVis_anVolumeFlags, 0, FVis_pVisData->nVolumeCount ); 
	}

	#if FANG_PLATFORM_GC | FANG_PLATFORM_XB
		FDS_StreamMgr.Update();
	#endif // FANG_PLATFORM_GC

	// Clear shadow count
	fvis_ClearNumShadows();

	// Increment the draw keys
	CFWorldMesh::IncDrawnKey();
	CFWorldPSGroup::IncDrawnKey();
}


//
//
//
void fvis_BlockOnStreamingData( void )
{
	_bBlockOnStreamingData = TRUE;
}


//
//
// Note: Make sure that fvis_ClearVolumeFlags is called once per frame before
// the first call to fvis_Draw.
void fvis_Draw( const CFMtx43A *pCamera, BOOL bDrawEntireWorld, u32 nFVisDataBufferSlot/*=0*/, BOOL bDrawDevShapes/*=TRUE*/ ) 
{
	FASSERT( nFVisDataBufferSlot < FVIS_MAX_BUFFERED_RENDERED_DATA_SLOTS );
	FRendererFogState_t PrevFogState;

	if ( !pCamera )
	{
		pCamera = &FXfm_pView->m_MtxR;
	}
	
	_nFVisCurrDataBufferSlot = nFVisDataBufferSlot;
	
	// Set the ambient world light
	fworld_Ambient_Set( &FVis_pVisData->AmbientLightColor, FVis_pVisData->fAmbientLightIntensity );
	
	// Build the display list
	if ( !_DL_Create( pCamera, bDrawEntireWorld ) )
	{
		return;
	}

	fmesh_ResetLightList();

	// Save previous frenderer fog state...
	frenderer_Fog_GetState( &PrevFogState );

	// Prepare to render the meshes
	frenderer_Push( FRENDERER_MESH, NULL );

	// Set the ambient world light
	//fworld_Ambient_Set( &FVis_pVisData->AmbientLightColor, FVis_pVisData->fAmbientLightIntensity );

	// Set frenderer fog mode...
	if ( _bEnableFog && (FVis_pVisData->fFogStartZ != 0.0f || FVis_pVisData->fFogEndZ != 0.0f) ) 
	{
		// Enable fog...
		frenderer_Fog_Enable( TRUE );
		frenderer_Fog_SetRange( FVis_pVisData->fFogStartZ, FVis_pVisData->fFogEndZ );
		frenderer_Fog_SetMotif( &FVis_pVisData->FogMotif );
	} 
	else 
	{
		// Disable fog...
		frenderer_Fog_Enable( FALSE );
	}
	
	////////////////////////////////////
	//
	// DRAW THE MESHES
	//
	////////////////////////////////////

	_nDrawMeshCount = 0;
	
	fmesh_FlushDrawPrep();
	
	if ( _bBlockOnStreamingData )
	{
		FDS_StreamMgr.BlockOnStreaming();
		_bBlockOnStreamingData = FALSE;
	}

	frs_FlushRenderLists();

//	_DrawLiquidVolumes();

	// Restore previous frenderer fog state...
	frenderer_Fog_SetState( &PrevFogState );

	frenderer_Pop();

	#if !FANG_PRODUCTION_BUILD
		if ( FRenderer_bDrawBoneInfo )
		{
			u32 nMeshIndex;

			for ( nMeshIndex = 0; nMeshIndex < FVis_anVisibleMeshCount[nFVisDataBufferSlot]; ++nMeshIndex )
			{
				FVis_papVisibleMeshes[nFVisDataBufferSlot][nMeshIndex]->DrawBoneInfo();
			}
		}
	#endif

	_RenderLightFX( pCamera, _nFVisCurrDataBufferSlot );
	
	fmesh_ResetLightList();

#if _ENABLE_VOLUME_DRAWING
	if ( FVis_bDrawVisibility || FVis_bDrawCellsOnly )
	{
		_DrawVolumePlanes();
	}
#endif

#if FMESH_COLL_ENABLE_COLLISION_DRAWS
	fmesh_Coll_DrawCollisions();
#endif
	
	if ( FVis_bShowDrawStats )
	{
/*
		u32 i;
		for ( i = 0; i < FVis_pVisData->nVolumeCount; i++ )
		{
			ftext_DebugPrintf( 0.8f, 0.10f + (i * 0.02f), "%d, %d, %d, %d, %d", 
				FVis_pVisData->paVolumes[i].anTotalTrackerCount[0],
				FVis_pVisData->paVolumes[i].anTotalTrackerCount[1], 
				FVis_pVisData->paVolumes[i].anTotalTrackerCount[2],
				FVis_pVisData->paVolumes[i].anTotalTrackerCount[3],
				FVis_pVisData->paVolumes[i].anTotalTrackerCount[4] );
		}
*/
	}
	
	#if !FANG_PRODUCTION_BUILD
		if ( bDrawDevShapes )
		{
			fdraw_RenderDevShapes();
		}
	#endif
}


//
//
//
void fvis_FrameEnd( void )
{
	#if !FANG_PRODUCTION_BUILD
		fdraw_ClearDevShapes();
	#endif
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Creation of the display lists
/////////////////////////////////////////////////////////////////////////////////////////////


#define _MAX_BUFFERED_VOLUME_LIGHTS		64
static CFWorldLight *apVolumeLights[_MAX_BUFFERED_VOLUME_LIGHTS];
static u32 _nVolumeLightCount;

#if _PORTAL_MIRRORS_ENABLED
	// Mirror rendering....
	static CFMtx43A *_pMirrorMtx;
	static CFVec3A _vMirrorNormal;
	static CFVec3A _vMirrorPosition;
	static u32 _nUseMirror;
#endif // _PORTAL_MIRRORS_ENABLED

static u32 _nLoopPrevention;

//
// _DL_Create
//
static BOOL _DL_Create( const CFMtx43A *pCameraMtx, BOOL bDrawEntireWorld )
{
	// Release the prior display lists
	FVis_anVisibleLightCount[_nFVisCurrDataBufferSlot] = 0;
	FVis_anVisibleMeshCount[_nFVisCurrDataBufferSlot] = 0;
	_nTotalMeshesDrawn = 0;
	_ActiveOccluderCount = 0;
	_nLoopPrevention = 0;

#if _PORTAL_MIRRORS_ENABLED
	_nMirrorsInViewCount = 0;
#endif // _PORTAL_MIRRORS_ENABLED

	_nMaterialDrawnKey++;
	
	// Record the camera
	FVis_mtxRenderCamera.Set( *pCameraMtx );
	FVis_mtxInvRenderCamera.ReceiveInverse( *pCameraMtx );
	FVis_aLastRenderCamera[_nFVisCurrDataBufferSlot].Set( *pCameraMtx );

	// Setup the current directional light weight
	_paDirLights[0].ComputeColor();
	FVis_fCurrDirLightWeight = _paDirLights[0].m_ScaledColor.ColorRGB.GetWeight();
		
	// Get the cell that the camera is in
	if ( pCameraMtx && pCameraMtx->m_vPos != _vCameraPos )
	{
		// If the camera has moved, we need to get the new volume
		_fCameraDepthInVolume = fworld_GetVolumeContainingPoint( &pCameraMtx->m_vPos, &_pCamVolume );
		_vCameraPos = pCameraMtx->m_vPos;
	}

	if ( !_pCamVolume || _pCamVolume->nVolumeID == 0 )
	{
		// If the camera is in no volume or the "slop bucket" volume, then draw the whole world
		bDrawEntireWorld = TRUE;
	}
	
	// Get the current viewport...
	FVis_pCurrentViewport = fviewport_GetActive();
	if ( FVis_pCurrentViewport == NULL )
	{
		// No viewport. Don't render the world...
		return FALSE;
	}

	_fFarPlaneDist = FVis_pCurrentViewport->fFarZ;

	// Transform frustum plane normals into world space and push them into the normal stack...
	u32 i;
	CFVec3A vTemp;
	FVis_VPNormalStack.ResetStack( &_vCameraPos );
	FVis_VPNormalStack.PushBase();
	for ( i = 0; i < 4; i++ )  
	{
		vTemp.v3 = pCameraMtx->m44.MultDir( FVis_pCurrentViewport->aaaUnitNorm[FVIEWPORT_DIR_IN][FVIEWPORT_SPACE_VS][i] );
		FVis_VPNormalStack.AddNormal( vTemp );
	}

	_vNearPlaneNormal = pCameraMtx->m44.MultDir( FVis_pCurrentViewport->aaaUnitNorm[FVIEWPORT_DIR_IN][FVIEWPORT_SPACE_VS][5] );
	_vFarPlaneNormal = pCameraMtx->m44.MultDir( FVis_pCurrentViewport->aaaUnitNorm[FVIEWPORT_DIR_IN][FVIEWPORT_SPACE_VS][4] );
	_vFarPlanePoint = pCameraMtx->m_vZ.v3*FVis_pCurrentViewport->fFarZ + pCameraMtx->m_vPos.v3;
	FVis_fLastVPFOVDistMod = FVis_pCurrentViewport->fTanHalfFOVX / FVis_fDefaultTanHalfFOVX;
	
	#if _PORTAL_MIRRORS_ENABLED
		// Initially, we are not in a mirror state
		_nUseMirror = 0;
	#endif // _PORTAL_MIRRORS_ENABLED

	//Prepare for adding volumes by clearing shadow volume light data.
	fshadow_ClearVolumeList();
	//
	
	_nDrawKey = CFWorldKey::OpenKey();
	if ( bDrawEntireWorld )
	{
		// Add all volumes to the display list
		for ( i = 0; i < FVis_pVisData->nVolumeCount; i++ )
		{
			_DL_AddVolume( &FVis_pVisData->paVolumes[i] );
		}
	}
	else
	{
		#if _LIMIT_PORTAL_TRAVERSAL_COUNT
			_ResetPortalBits();
		#endif

		// Put the "slop bucket" volume in the display list
		_DL_AddVolume( &FVis_pVisData->paVolumes[FVIS_SLOP_BUCKET_ID] );
		
		// Start recursively building the display list
		_DL_WalkVisibility( _pCamVolume, 0xffff );
	}
	CFWorldKey::CloseKey( _nDrawKey );
	
	// We shouldn't have any normals straggling in the normal stack
	u16 nAbandonedNormals = FVis_VPNormalStack.PopBase();
	FASSERT( nAbandonedNormals == 0 );
	
	return TRUE;
}


//
//
//
FINLINE BOOL _Occlusion_PointIsInFrustum( _AntiPortalOccluder *pOccluder, CFVec3 *pPoint, f32 fRadius = 0.f )
{
	u32 i;
	
	FASSERT( pOccluder && pPoint );

	// Check all of the normals for the occluder to make sure the point is contained
	for ( i = 0; i < pOccluder->nNormalCount; i++ )
	{
		CFVec3 vDiff = *pPoint - _vCameraPos.v3;
		
		// We also could have a radius submitted if someone is checking for occlusion of a sphere
		if ( vDiff.Dot( pOccluder->vFrustrumNormals[i].v3 ) < fRadius )
		{
			return FALSE;
		}
	}
	
	return TRUE;
}


//
//
//
static BOOL _Occlusion_SphereIsOccluded( CFSphere *pSphere )
{
	u32 i;
	
	FASSERT( pSphere );
	
	// Run through all of the occluders
	for ( i = 0; i < _ActiveOccluderCount; i++ )
	{
		// First check to see if the sphere is on the occluded side of the occlusion plane
		CFVec3 vDiff = _Occluders[i].vPlanePoint_WS.v3 - pSphere->m_Pos;
		if ( vDiff.Dot( _Occluders[i].vPlaneNormal_WS.v3 ) < pSphere->m_fRadius )
		{
			// Part or all of sphere is on the visible side of the occluder
			continue;
		}
		
		// Next check the sphere against the frustum of the occluder
		if ( !_Occlusion_PointIsInFrustum( &_Occluders[i], &pSphere->m_Pos, pSphere->m_fRadius) )
		{
			continue;			
		}
		
		// This sphere is occluded
		return TRUE;
	}
	
	// We could not occlude the sphere with the current set of occluders
	return FALSE;
}


//
//
//
static BOOL _Occlusion_PortalIsOccluded( FVisPortal_t *pPortal )
{
	u32 i, ii;
	
	FASSERT( pPortal );
	
	// Run through all of the occluders
	for ( i = 0; i < _ActiveOccluderCount; i++ )
	{
		for ( ii = 0; ii < pPortal->nVertCount; ii++ )
		{
			// First check to see if the point is on the occluded side of the occlusion plane
			CFVec3 vDiff = _Occluders[i].vPlanePoint_WS.v3 - pPortal->avVertices[ii];
			if ( vDiff.Dot( _Occluders[i].vPlaneNormal_WS.v3 ) < 0.f )
			{
				// The point is on the visible side of the occluder
				break;
			}
			
			// Next check the sphere against the frustum of the occluder
			if ( !_Occlusion_PointIsInFrustum( &_Occluders[i], &pPortal->avVertices[ii] ) )
			{
				break;
			}
		}
		
		if ( ii == pPortal->nVertCount )
		{
			return TRUE;
		}
	}
	
	return FALSE;
}


//
//
//
static f32 _Occlusion_GetPortalScreenArea( FVisPortal_t *pPortal, f32 fViewDot )
{
	CFVec3 vDiff;
	
	// Calculate distance from the camera
	vDiff = pPortal->avVertices[0] - _vCameraPos.v3;
	f32 fDistFromCam = vDiff.Mag();
	if ( fDistFromCam == 0.f )
	{
		return 0.f;
	}

	// Calculate the area of the triangle in the view frustum
	f32 fEdge0, fEdge1, fEdge2, fSP;
	vDiff = pPortal->avVertices[0] - pPortal->avVertices[1];
	fEdge0 = vDiff.Mag();
	vDiff = pPortal->avVertices[1] - pPortal->avVertices[2];
	fEdge1 = vDiff.Mag();
	vDiff = pPortal->avVertices[0] - pPortal->avVertices[2];
	fEdge2 = vDiff.Mag();
	
	// Calculate the semi-perimeter:
	fSP = 0.5f * (fEdge0 + fEdge1 + fEdge2);
	f32 fTriArea = fmath_Sqrt( fSP * (fSP - fEdge0) * (fSP - fEdge1) * (fSP - fEdge2));
	
	vDiff = pPortal->avVertices[0] - pPortal->avVertices[3];
	fEdge0 = vDiff.Mag();
	vDiff = pPortal->avVertices[2] - pPortal->avVertices[3];
	fEdge1 = vDiff.Mag();
	vDiff = pPortal->avVertices[0] - pPortal->avVertices[2];
	fEdge2 = vDiff.Mag();
	
	// Calculate the semi-perimeter:
	fSP = 0.5f * (fEdge0 + fEdge1 + fEdge2);
	fTriArea += fmath_Sqrt( fSP * (fSP - fEdge0) * (fSP - fEdge1) * (fSP - fEdge2));
	
	fTriArea *= (-fViewDot) * fmath_Inv( fDistFromCam );
	
	return fTriArea;
}
	
//
//
//
static void _Occlusion_ConsiderPortal( FVisPortal_t *pPortal, u32 nVolumeID )
{
	u32 i;
	CFVec3 vDiff;
	u32 nVolumePortalIdx = 0;
	BOOL bWindClockWise = TRUE;
	
	// Check against the camera position to see if it is behind the viewport
	for ( i = 0; i < pPortal->nVertCount; i++ )
	{
/*	
		if ( FVis_VPNormalStack.PointIsVisible( &pPortal->avVertices[i] ) )
		{
			break;
		}
*/
		vDiff = pPortal->avVertices[i] - _vCameraPos.v3;
		if ( vDiff.Dot( FVis_mtxRenderCamera.m_vFront.v3 ) > 0 )
		{
			break;
		}
	}
	
	// If we cycled through all verts and they were behind camera view, bail
	if ( i == pPortal->nVertCount )
	{
		return;
	}
			
	// Is the current normal facing the camera?	
	
	// Establish a unitized vector to the camera
	f32 fDist = vDiff.Mag2();
	if ( fDist < 0.01f )
	{
		return;
	}
	vDiff *= fmath_InvSqrt( fDist );
	
	CFVec3 vPortalNormal( pPortal->vNormal );
	f32 fDot = vPortalNormal.Dot( vDiff );
	if ( fDot > 0 )
	{
		if ( pPortal->nFlags & FVIS_PORTAL_FLAG_ONE_WAY )
		{
			return;
		}
		
		// Reverse the normal so that our calculations will be correct
		vPortalNormal = -vPortalNormal;
		fDot = -fDot;
		bWindClockWise = FALSE;
	}
	
	// Figure out if we should put this occluder in the occluders list
	_AntiPortalOccluder *pOccluder = NULL;
	f32 fNewScreenArea = -FMATH_MAX_FLOAT;
	if ( _ActiveOccluderCount == _MAX_ANTIPORTALS_IN_VIEW )
	{
		// Calculate the occluder screen area
		fNewScreenArea = _Occlusion_GetPortalScreenArea( pPortal, fDot );
		
		// Find out if we have a worse occluder in the list
		for ( i = 0; i < _ActiveOccluderCount; i++ )
		{
			if ( _Occluders[i].fScreenArea == -FMATH_MAX_FLOAT )
			{
				_Occluders[i].fScreenArea = _Occlusion_GetPortalScreenArea( pPortal, fDot );
			}
			
			if ( _Occluders[i].fScreenArea < fNewScreenArea )
			{
				pOccluder = &_Occluders[i];
				break;
			}
		}
		
		if ( pOccluder == NULL )
		{
			return;
		}
	}
	else
	{
		// Put the occluder at the next open slot in the occluder list
		pOccluder = &_Occluders[ _ActiveOccluderCount++ ];
	}

	// Fill out the occluder info
	pOccluder->fScreenArea = fNewScreenArea;
	pOccluder->nNormalCount = 0;
	pOccluder->vPlaneNormal_WS.Set( vPortalNormal );
	pOccluder->vPlanePoint_WS.Set( pPortal->avVertices[0] );

	// Calculate the normals	
	u32 iPriorVert = pPortal->nVertCount - 1;
	CFVec3 vDiff1, vDiff2;
	f32 fCrossMag;
	if ( bWindClockWise )
	{
		// If we're dealing with the portal as seen from volume 0, our
		// winding order for the verts is clockwise
		for ( i = 0; i < pPortal->nVertCount; i++ )
		{
			vDiff1 = pPortal->avVertices[i] - pPortal->avVertices[iPriorVert];
			vDiff2 = pPortal->avVertices[i] - _vCameraPos.v3;
			if ( vDiff1.Mag2() < 0.001f || vDiff2.Mag2() < 0.001f )
			{
				iPriorVert = i;
				continue;
			}
			pOccluder->vFrustrumNormals[pOccluder->nNormalCount].v3 = vDiff1.Cross( vDiff2 );
			fCrossMag = pOccluder->vFrustrumNormals[pOccluder->nNormalCount].MagSq();
			if ( fCrossMag < 0.001f )
			{
				iPriorVert = i;
				continue;
			}			
			pOccluder->vFrustrumNormals[pOccluder->nNormalCount].Mul( fmath_InvSqrt(fCrossMag) );
			pOccluder->nNormalCount++;
			iPriorVert = i;
		}
	}
	else
	{
		// If we're dealing with the portal as seen from volume 1, our
		// winding order for the verts is counter-clockwise
		for ( i = 0; i < pPortal->nVertCount; i++ )
		{
			vDiff1 = pPortal->avVertices[iPriorVert] - pPortal->avVertices[i];
			vDiff2 = pPortal->avVertices[iPriorVert] - _vCameraPos.v3;
			if ( vDiff1.Mag2() < 0.001f || vDiff2.Mag2() < 0.001f )
			{
				iPriorVert = i;
				continue;
			}
			pOccluder->vFrustrumNormals[pOccluder->nNormalCount].v3 = vDiff1.Cross( vDiff2 );
			fCrossMag = pOccluder->vFrustrumNormals[pOccluder->nNormalCount].MagSq();
			if ( fCrossMag < 0.001f )
			{
				iPriorVert = i;
				continue;
			}			
			pOccluder->vFrustrumNormals[pOccluder->nNormalCount].Mul( fmath_InvSqrt(fCrossMag) );
			pOccluder->nNormalCount++;
			iPriorVert = i;
		}
	}
}


//
//	This function fills out the list of lights that fall within 
//	the view frustum in the currently displayed volume
//
static BOOL _DL_GenerateVolumeLights( FVisVolume_t *pVolume )
{
	CFWorldLight *pWorldLight;
	_nVolumeLightCount = 0;
    // Circulate through all of the intersecting lights buffering those that affect the volume

#if FANG_DEBUG_BUILD
	u32 nKeyIndex = CFWorldKey::OpenKey();
	if( nKeyIndex == -1 ) 
	{
		DEVPRINTF( "_DL_GenerateVolumeLights(): No more world keys available.\n" );
		return FALSE;
	}
#endif

	FLinkRoot_t *pRootInt = &pVolume->aTrackerIntersects[FWORLD_TRACKERTYPE_LIGHT];

	CFWorldIntersect *pTestInt = NULL;
	while ( (pTestInt = (CFWorldIntersect *)flinklist_GetNext( pRootInt, pTestInt )) ) 
	{
		pWorldLight = (CFWorldLight *)pTestInt->GetTracker();

#if FANG_DEBUG_BUILD
		// Check to see if we've already visited this tracker...
		if ( pWorldLight->m_VisitedKey.HaveVisited( nKeyIndex ) ) 
		{
			// We shouldn't have a light added to a volume more than once
			FASSERT_NOW;
		}

		// Flag as visited...
		pWorldLight->m_VisitedKey.FlagAsVisited( nKeyIndex );
#endif

		if ( !(pWorldLight->m_Light.m_nFlags & FLIGHT_FLAG_ENABLE) ) 
		{
			// Light is disabled...
			continue;
		}

		CFSphere spTracker( pWorldLight->GetBoundingSphere() );
		if ( fviewport_TestSphere_WS( FVis_pCurrentViewport, &spTracker, FVIEWPORT_PLANESMASK_ALL ) == -1 )
		{
			continue;
		}

		if ( !pWorldLight->m_VisitedKey.HaveVisited( _nDrawKey ) )
		{
			CFVec3A vTemp;
			vTemp.v3 = _vCameraPos.v3 - spTracker.m_Pos;
			f32 fDistanceSq = vTemp.MagSq();
			u32 nIdx = FVis_anVisibleLightCount[_nFVisCurrDataBufferSlot];
			pWorldLight->UpdateOrientationAndPosition();
			if ( nIdx < FVIS_MAX_VISIBLE_LIGHTS_LIST_COUNT )
			{
				pWorldLight->m_VisitedKey.FlagAsVisited( _nDrawKey );
				FVis_paVisibleLights[_nFVisCurrDataBufferSlot][ nIdx ].pLight = pWorldLight;
				FVis_paVisibleLights[_nFVisCurrDataBufferSlot][ nIdx ].fDistFromCameraSq = fDistanceSq;
				FVis_paVisibleLights[_nFVisCurrDataBufferSlot][ nIdx ].bOriginVisible = (FVis_VPNormalStack.PointIsVisible( &spTracker.m_Pos, 0.f  ) != FMATH_MAX_FLOAT);
				pWorldLight->m_nVisibileLightIdx = nIdx;
				FVis_anVisibleLightCount[_nFVisCurrDataBufferSlot]++;
			}
			else
			{
				// See if this light is closer than any existing light
				u32 i;
				for ( i = 0; i < FVis_anVisibleLightCount[_nFVisCurrDataBufferSlot]; i++ )
				{
					if ( FVis_paVisibleLights[_nFVisCurrDataBufferSlot][ i ].fDistFromCameraSq > fDistanceSq )
					{
						pWorldLight->m_VisitedKey.FlagAsVisited( _nDrawKey );
						FVis_paVisibleLights[_nFVisCurrDataBufferSlot][ i ].pLight = pWorldLight;
						FVis_paVisibleLights[_nFVisCurrDataBufferSlot][ i ].fDistFromCameraSq = fDistanceSq;
						FVis_paVisibleLights[_nFVisCurrDataBufferSlot][ i ].bOriginVisible = (FVis_VPNormalStack.PointIsVisible( &spTracker.m_Pos, 0.f  ) != FMATH_MAX_FLOAT);
						pWorldLight->m_nVisibileLightIdx = i;
					}
				}
			}
		}
		else
		{
			if ( !FVis_paVisibleLights[_nFVisCurrDataBufferSlot][ pWorldLight->m_nVisibileLightIdx ].bOriginVisible )
			{
				FVis_paVisibleLights[_nFVisCurrDataBufferSlot][ pWorldLight->m_nVisibileLightIdx ].bOriginVisible = (FVis_VPNormalStack.PointIsVisible( &spTracker.m_Pos, 0.f  ) != FMATH_MAX_FLOAT);
			}
		}
		
		if ( _nVolumeLightCount == _MAX_BUFFERED_VOLUME_LIGHTS )
		{
			continue;
		}
			
		apVolumeLights[_nVolumeLightCount++] = pWorldLight;
	}
	
#if FANG_DEBUG_BUILD
	CFWorldKey::CloseKey( nKeyIndex );
#endif
	
	return TRUE;
}

static FViewportPlanesMask_t _VolumeCrossPlanes;

//
//
//
static BOOL _DL_AddVolume( FVisVolume_t *pVolume )
{
	// Ensure that the volume is visible
	_VolumeCrossPlanes = fviewport_TestSphere_WS( FVis_pCurrentViewport, &pVolume->spBoundingWS, FVIEWPORT_PLANESMASK_ALL );
	if ( _VolumeCrossPlanes == -1 )
	{
		return FALSE;
	}
	
	// Determine the visible lights in this volume
	_DL_GenerateVolumeLights( pVolume );

	CFWorldTracker *pTracker;
	FLinkRoot_t *pRootIntersect;
	CFWorldIntersect *pIntersect;

	// Add in the meshes that are in this volume
#if !FANG_PRODUCTION_BUILD	
	if ( !FVis_bDontDrawObjectGeo )
#endif // !FANG_PRODUCTION_BUILD
	{
		pRootIntersect = &pVolume->aTrackerIntersects[FWORLD_TRACKERTYPE_MESH];
		pIntersect = NULL;
		while ( (pIntersect = (CFWorldIntersect *)flinklist_GetNext( pRootIntersect, pIntersect )) ) 
		{
			pTracker = pIntersect->GetTracker();

			// See if we have already flagged this to be drawn
			if ( pTracker->m_VisitedKey.HaveVisited( _nDrawKey ) )
			{
				continue;
			}
			
			// Check to see if this is flagged not to be drawn
			if ( ((CFWorldMesh *)pTracker)->m_nFlags & FMESHINST_FLAG_DONT_DRAW )
			{
				// Flag this as drawn so we don't check it again
				pTracker->m_VisitedKey.FlagAsVisited( _nDrawKey );
				continue;
			}
		
			_DL_AddMesh( pTracker );
		}
	}
	
	// Add in the point sprites that are in this volume
#if !FANG_PRODUCTION_BUILD
	if ( !FVis_bDontDrawPointSprites )
#endif // !FANG_PRODUCTION_BUILD
	{
		pRootIntersect = &pVolume->aTrackerIntersects[FWORLD_TRACKERTYPE_PSGROUP];
		pIntersect = NULL;
		while ( (pIntersect = (CFWorldIntersect *)flinklist_GetNext( pRootIntersect, pIntersect )) ) 
		{
			pTracker = pIntersect->GetTracker();

			// See if we have already flagged this to be drawn
			if ( pTracker->m_VisitedKey.HaveVisited( _nDrawKey ) )
			{
				continue;
			}
			
			CFWorldPSGroup *pWorldPSGroup = (CFWorldPSGroup *)pTracker;
			if ( ((CFWorldPSGroup *)pTracker)->m_nPSFlags & FPSPRITE_FLAG_DONT_DRAW ) 
			{
				// Flag this as drawn so we don't check it again
				pTracker->m_VisitedKey.FlagAsVisited( _nDrawKey );
				continue;
			}

			_DL_AddPointSprite( pTracker );
		}
	}

	// If we haven't already added this volume to the draw list, setup all of the appropriate data
	if ( !pVolume->Key.HaveVisited( _nDrawKey ) )
	{
		pVolume->Key.FlagAsVisited( _nDrawKey );

		// Set the volume flags
		if ( !FVis_bInRenderTarget || FVis_bAllowLiquid_RenderTarget )
		{
			FVis_anVolumeFlags[pVolume->nVolumeID] = FVIS_VOLUME_IN_ACTIVE_LIST | FVIS_VOLUME_IN_VISIBLE_LIST;
		}

		if ( !pVolume->pWorldGeo )
		{
			return TRUE;
		}

		pVolume->pWorldGeo->CacheMeshData( 0 );
		pVolume->pWorldGeo->DrawPrep( _VolumeCrossPlanes, TRUE, FALSE );

		// Clear the mesh's light list
		pVolume->pWorldGeo->ResetLightList();

		fshadow_SaveVolumeLightList(apVolumeLights, _nVolumeLightCount, pVolume->pWorldGeo);

		// Buffer the volume lights that might affect the mesh
		u32 i;
		for ( i = 0; i < _nVolumeLightCount; i++ )
		{
			CFWorldLight *pWorldLight = apVolumeLights[i];
			FASSERT( pWorldLight );
			
			if ( pWorldLight->m_Light.m_nFlags & FLIGHT_FLAG_NOLIGHT_TERRAIN ) 
			{
				// Light is not permitted to light the terrain...
				continue;
			}
			
			pVolume->pWorldGeo->ConsiderLightForRender( &pWorldLight->m_Light, &pVolume->spBoundingWS );
		}
	}
	else if ( !pVolume->pWorldGeo )
	{
		return TRUE;
	}

	// Add in the materials that are visible
	CFSphere spTest;
	FMeshMaterial_t *pMat;
	FMeshMaterial_t *pLastMat = &pVolume->pWorldGeo->m_pMesh->aMtl[pVolume->pWorldGeo->m_pMesh->nMaterialCount];
	f32 fMeshRadius = pVolume->pWorldGeo->m_pMesh->BoundSphere_MS.m_fRadius;
	for ( pMat = pVolume->pWorldGeo->m_pMesh->aMtl; pMat != pLastMat; pMat++ )
	{
		if ( pMat->__PAD == _nMaterialDrawnKey )
		{
			// This material is already being drawn, this render
			continue;
		}

		f32 fDistFromCam;
		if ( _VolumeCrossPlanes != FVIEWPORT_PLANESMASK_NONE && pMat->nCompressedRadius < 235 )
		{
			#if FPERF_ENABLE
				FPerf_nVolMeshMtlFrstmTested++;
			#endif

			// Is this material in the current viewport?
			spTest.m_fRadius = (pMat->nCompressedRadius + 1) * (1.f/255.f) * fMeshRadius;
			if ( FVis_VPNormalStack.PointIsVisible( &pMat->vAverageVertPos, spTest.m_fRadius, &fDistFromCam ) == CFVisNormalStack::OUTSIDE_FRUSTUM_BOUNDS )
			{
				// Outside the viewport, so skip it
				#if FPERF_ENABLE
					FPerf_nVolMeshMtlCullCount++;
				#endif
				continue;
			}

			// This material is in the viewport, so it should be added to the render list

			#if FANG_PLATFORM_XB
				// Xbox doesn't allow us to change the clipping mode, so don't worry about it
				frs_AddVolumeMeshMaterial( pVolume->pWorldGeo, pMat, _VolumeCrossPlanes, fDistFromCam * FVis_fLastVPFOVDistMod );
			#else // !FANG_PLATFORM_XB
				// Check to see if we need to clip this material
				spTest.m_Pos = pMat->vAverageVertPos;
				u8 nMatCrossesPlanesMask = (u8)fviewport_TestSphere_WS( FVis_pCurrentViewport, &spTest, FVIEWPORT_PLANESMASK_ALL );
				#if FPERF_ENABLE
					if ( nMatCrossesPlanesMask == FVIEWPORT_PLANESMASK_NONE )
					{
						// We were able to remove clipping from this material
						FPerf_nVolMeshMtlUnclipCount++;
					}
				#endif
				frs_AddVolumeMeshMaterial( pVolume->pWorldGeo, pMat, nMatCrossesPlanesMask, fDistFromCam * FVis_fLastVPFOVDistMod );
			#endif
		}
		else
		{
			fDistFromCam = (pMat->vAverageVertPos - _vCameraPos.v3).Mag2();
			if ( fDistFromCam > 0.0001f )
			{
				fDistFromCam = fmath_Sqrt( fDistFromCam );
			}
			frs_AddVolumeMeshMaterial( pVolume->pWorldGeo, pMat, _VolumeCrossPlanes, fDistFromCam * FVis_fLastVPFOVDistMod );
		}

		// Mark this material as drawn
		pMat->__PAD = _nMaterialDrawnKey;
	}

	return TRUE;
}


//
//	This function is a callback from the world system.
//
static void _DL_AddMesh( CFWorldTracker *pTracker )
{
	FASSERT( pTracker );
	FASSERT( pTracker->GetType() == FWORLD_TRACKERTYPE_MESH );

	CFWorldMesh *pMesh = (CFWorldMesh *)pTracker;

	CFSphere spTracker( pTracker->GetBoundingSphere() );

	#if _PORTAL_MIRRORS_ENABLED
		if ( _nUseMirror )
		{
			// Is the object too close to or behind the mirror?
			CFVec3A vTemp;
			vTemp.v3 = _vMirrorPosition.v3 - spTracker.m_Pos;
			f32 fDistToMirrorPlane = vTemp.Dot( _vMirrorNormal );
			if ( fDistToMirrorPlane > -0.5f )
			{
				return;
			}
			
			// If we have a mirror, we want to project the object into view space as seen
			// through the mirror so that we get correct viewport clipping
			spTracker.m_Pos = _pMirrorMtx->m44.MultPoint( spTracker.m_Pos );
		}
	#endif // _PORTAL_MIRRORS_ENABLED

	// Check the mesh against the frustum
	f32 fDistFromCam;
	CFVisNormalStack::FrustumCheckResults_e nObjectFrustumResults = FVis_VPNormalStack.PointIsVisible( &spTracker.m_Pos, spTracker.m_fRadius, &fDistFromCam );
	if ( nObjectFrustumResults == CFVisNormalStack::OUTSIDE_FRUSTUM_BOUNDS || fDistFromCam  > _fFarPlaneDist || fDistFromCam > pMesh->m_fCullDist )
	{
		pMesh->CacheMeshData( 1 );
		return;
	}
	if ( fDistFromCam < FVis_fMinRenderDist )
	{
		return;
	}

	// Flag this as drawn
	pTracker->m_VisitedKey.FlagAsVisited( _nDrawKey );

	if ( _ActiveOccluderCount && _Occlusion_SphereIsOccluded( &spTracker ) )
	{
		pMesh->CacheMeshData( 1 );
		return;
	}
	
	// Set the clipping parameters for this object
	FViewportPlanesMask_t ObjectCrossPlanes;
	#if FANG_PLATFORM_XB
		ObjectCrossPlanes = FVIEWPORT_PLANESMASK_NONE;
	#else
		// No clipping option on XB, but other platforms will want to optimize based on clipping
		if ( pMesh->m_nFlags & FMESHINST_FLAG_FORCE_CLIP )
		{
			// This tracker has been flagged to always be clipped
			ObjectCrossPlanes = FVIEWPORT_PLANESMASK_ALL;
		}
		else
		{
			ObjectCrossPlanes = fviewport_TestSphere_WS( FVis_pCurrentViewport, &spTracker, FVIEWPORT_PLANESMASK_ALL );
			if ( ObjectCrossPlanes == -1 )
			{
				return;
			}
		}
	#endif
	
	// This mesh is visible so add it into the display list and generate lighting info

	f32 fScreenRadius = fviewport_ComputeScreenRadius_WS( FVis_pCurrentViewport, &FVis_mtxInvRenderCamera, &spTracker ) * (f32)FVis_pCurrentViewport->nWidth;
	if ( !pMesh->WasDrawnThisFrame() || pMesh->GetPixelRadiusLastDrawn() < fScreenRadius )
	{
		pMesh->SetPixelRadiusLastDrawn( (u32)fScreenRadius );
	}
	
	// Add the tracker to the list of visible meshed
	if ( FVis_anVisibleMeshCount[_nFVisCurrDataBufferSlot] < FVIS_MAX_VISIBLE_MESH_LIST_COUNT )
	{
		FVis_papVisibleMeshes[_nFVisCurrDataBufferSlot][ FVis_anVisibleMeshCount[_nFVisCurrDataBufferSlot] ] = pMesh;
		FVis_anVisibleMeshCount[_nFVisCurrDataBufferSlot]++;
	}
	
	// Make sure we can successfully prep the mesh for drawing
	if ( pMesh->DrawPrep( ObjectCrossPlanes, TRUE, FALSE ) == -1 )
	{
		return;
	}

	pMesh->CacheMeshData( 0 );
	
//	ftext_DebugPrintf( 0.1f, 0.1f + (_nTotalMeshesDrawn++ * 0.02f), "%s", pMesh->m_pMesh->szName );

	if (pMesh->m_bReflect)
	{
		if ( fDistFromCam > 0.1f)
		{
			f32 fSize = pMesh->GetBoundingSphere().m_fRadius * fmath_Inv(fDistFromCam);
			pMesh->m_pReflectTex = fsh_GetSReflectTarget(fSize, pMesh);
		}
		else
		{
			f32 fSize = pMesh->GetBoundingSphere().m_fRadius * 10.0f;
			pMesh->m_pReflectTex = fsh_GetSReflectTarget(fSize, pMesh);	
		}
	}
	else
	{
		pMesh->m_pReflectTex = NULL;
	}

	// Clear the mesh's light list
	pMesh->ResetLightList();
	
	u32 i;

	// Set up array of world lights affecting the mesh
	if ( !(pMesh->m_nFlags & FMESHINST_FLAG_NOLIGHT_DYNAMIC) ) 
	{
		// Add the directional lights to the mesh
		for ( i = 0; i < _nDirLightCount; i++ ) 
		{
			pMesh->ConsiderLightForRender( &_paDirLights[i], &spTracker );
		}

        // Circulate through all of the intersecting lights buffering those that affect the mesh
#if FANG_DEBUG_BUILD
		u32 nKeyIndex = CFWorldKey::OpenKey();
		if( nKeyIndex == -1 ) 
		{
			DEVPRINTF( "_DL_AddMesh(): No more world keys available.\n" );
		}
		else
#endif // FANG_DEBUG_BUILD
		{
			CFWorldIntersect *pSrcIntersect = NULL;
			while( (pSrcIntersect = pMesh->GetNextIntersect( pSrcIntersect )) ) 
			{
				// Each mesh intersect represents one volume that this object most likely intersects.

				FVisVolume_t *pVolumeInt = pSrcIntersect->GetVolume();
				FLinkRoot_t *pRootInt = &pVolumeInt->aTrackerIntersects[FWORLD_TRACKERTYPE_LIGHT];

				CFWorldIntersect *pTestInt = NULL;
				while ( (pTestInt = (CFWorldIntersect *)flinklist_GetNext( pRootInt, pTestInt )) ) 
				{
					CFWorldLight *pWorldLight = (CFWorldLight *)pTestInt->GetTracker();

#if FANG_DEBUG_BUILD
					// Check to see if we've already visited this tracker...
					if ( pWorldLight->m_VisitedKey.HaveVisited( nKeyIndex ) ) 
					{
						// We've already visited the intersecting tracker, so let's skip it...
						continue;
					}

					// Flag as visited...
					pWorldLight->m_VisitedKey.FlagAsVisited( nKeyIndex );
#endif // FANG_DEBUG_BUILD

					if ( !(pWorldLight->m_Light.m_nFlags & FLIGHT_FLAG_ENABLE) ) 
					{
						// Light is disabled...
						continue;
					}

					if ( spTracker.IsIntersecting( pWorldLight->GetBoundingSphere() ) )
					{
						pMesh->ConsiderLightForRender( &pWorldLight->m_Light, &spTracker );
					}
				}
			}
		}
#if FANG_DEBUG_BUILD
		CFWorldKey::CloseKey( nKeyIndex );
#endif // FANG_DEBUG_BUILD
	} 
	
	// Setup Ambient
	pMesh->m_FinalAmbientRGB = pMesh->m_AmbientRGB;
	if ( !(pMesh->m_nFlags & (FMESHINST_FLAG_NOLIGHT_AMBIENT|FMESHINST_FLAG_VERT_RADIOSITY|FMESHINST_FLAG_LM)) )
	{
		pMesh->m_FinalAmbientRGB += FWorld_AmbientMotif.ColorRGB;
	}
	pMesh->m_FinalAmbientRGB.Clamp1();
	
	frs_AddMeshInst( pMesh, ObjectCrossPlanes, fDistFromCam * FVis_fLastVPFOVDistMod, FVis_pCurrentViewport->fVPtoWindowPixelRatio );
	pMesh->FlagAsDrawn();
}


//
//	This function is a callback from the world system.
//
static void _DL_AddPointSprite( CFWorldTracker *pTracker )
{
	FASSERT( pTracker );
	FASSERT( pTracker->GetType() == FWORLD_TRACKERTYPE_PSGROUP );

	CFVec3A vDiff;
	CFWorldPSGroup *pWorldPSGroup = (CFWorldPSGroup *)pTracker;

	// Get position
	CFSphere spTracker( pWorldPSGroup->GetBoundingSphere() );
	CFVec3A vPos;
	vPos.Set( spTracker.m_Pos );//vPos( pWorldPSGroup->m_Xfm.m_MtxF.m_vPos ); 

	#if _PORTAL_MIRRORS_ENABLED
		if ( _nUseMirror )
		{
			_pMirrorMtx->MulPoint( vPos );
			spTracker.m_Pos = _pMirrorMtx->m44.MultPoint( spTracker.m_Pos );
		}
	#endif // _PORTAL_MIRRORS_ENABLED

	// Check the group against the frustum
	f32 fDistFromCam;
	if ( FVis_VPNormalStack.PointIsVisible( &spTracker.m_Pos, spTracker.m_fRadius, &fDistFromCam ) == CFVisNormalStack::OUTSIDE_FRUSTUM_BOUNDS )
	{
		return;
	}
	if ( fDistFromCam  > _fFarPlaneDist || fDistFromCam > pWorldPSGroup->m_fCullDist )
	{
		return;
	}
	
	u32 nCrossesPlanesMask = 0;
	if ( pWorldPSGroup->m_nPSFlags & FPSPRITE_FLAG_NOCLIP ) 
	{
		// The triangles in this object are guaranteed to never cross a frustum plane...
		nCrossesPlanesMask = 0;
	}

	// Flag this as drawn
	pTracker->m_VisitedKey.FlagAsVisited( _nDrawKey );

	frs_AddPSGroup( pWorldPSGroup, nCrossesPlanesMask, fDistFromCam * FVis_fLastVPFOVDistMod );
	pWorldPSGroup->FlagAsDrawn();
	
	return;
}


//
//	This function recursively walks the visible volumes starting with the
//	specified volume that the camera is in.  (We use that nPortalViewedFrom
//	index to prevent from traversing back into the same adjacent volume).
//
static void _DL_WalkVisibility( FVisVolume_t *pVolume, u32 nPortalViewedFrom )
{
	s32 i;
	FVisPortal_t *pPortal;
	CFVec3 vDiff;

	_nLoopPrevention++;
	if ( _nLoopPrevention > 250 )
	{
		DEVPRINTF( "_DL_WalkVisibility:: WARNING - Recursed into function > 250 times.  Potential infinite loop. Ceasing vis walk.\n" );
		return;
	}

	// First add in any occluders so we can take advantage of them
	for ( i = 0; i < pVolume->nPortalCount; i++ )
	{
		pPortal = &FVis_pVisData->paPortals[pVolume->paPortalIndices[i]];
		
		// We only consider anti-portals
		if ( pPortal->nFlags & FVIS_PORTAL_FLAG_ANTIPORTAL )
		{
			_Occlusion_ConsiderPortal( pPortal, pVolume->nVolumeID );
		}
	}

	if ( !_DL_AddVolume( pVolume ) ) 
	{
		return;
	}

	// Record portal center for later recursion prevention
	CFVec3 vViewPortalCenter, vViewPortalNormal;
	if ( nPortalViewedFrom != 0xffff )
	{
		pPortal = &FVis_pVisData->paPortals[pVolume->paPortalIndices[nPortalViewedFrom]];
		vViewPortalCenter = pPortal->spBoundingWS.m_Pos;
		if ( pPortal->anAdjacentVolume[1] != pVolume->nVolumeID )
		{
			vViewPortalNormal = pPortal->vNormal;
		}
		else
		{
			vViewPortalNormal = -pPortal->vNormal;
		}
	}
	else
	{
		vViewPortalCenter = _vCameraPos.v3;
		vViewPortalNormal = _vNearPlaneNormal;
	}
	
	// Check this volume's portals to determine visible adjacencies
	for ( i = 0; i < pVolume->nPortalCount; i++ )
	{
		// Skip the portal that we are looking into this room from
		if ( i == (s32)nPortalViewedFrom )
		{
			continue;
		}

		pPortal = &FVis_pVisData->paPortals[pVolume->paPortalIndices[i]];
		
		// If this portal is closed or an anti portal, then we can skip it.
		if ( (pPortal->nFlags & FVIS_PORTAL_FLAG_CLOSED) || !(pPortal->nFlags & (FVIS_PORTAL_FLAG_VISIBILITY | FVIS_PORTAL_FLAG_MIRROR)) )
		{
			continue;
		}
		
		// Determine our index in the portal connectivity description
		u16 nVolumePortalIdx, nAdjVolumePortalIdx;
		if ( pPortal->anAdjacentVolume[1] != pVolume->nVolumeID || (pPortal->nFlags & FVIS_PORTAL_FLAG_MIRROR) )
		{
			nVolumePortalIdx = 0;
			nAdjVolumePortalIdx = 1;
		}
		else
		{
			nAdjVolumePortalIdx = 0;
			nVolumePortalIdx = 1;
		}
		
		// Get a handy pointer to the adjacent volume
		FVisVolume_t *pAdjVolume = &FVis_pVisData->paVolumes[pPortal->anAdjacentVolume[nAdjVolumePortalIdx]];

		// Do not allow the system to traverse back into the camera volume		
		if ( !(pPortal->nFlags & FVIS_PORTAL_FLAG_MIRROR) && pAdjVolume == _pCamVolume )
		{
			continue;
		}

		// The dot product of the difference between the camera position and a vertex on portal and the 
		// normal of the portal should be negative  otherwise the portal is not facing the camera and should be skipped.
		vDiff = pPortal->avVertices[0] - _vCameraPos.v3;
		CFVec3 vPortalNormal( pPortal->vNormal );
		if ( nVolumePortalIdx == 1 )
		{
			vPortalNormal = -vPortalNormal;
		}
		if ( vDiff.Dot( vPortalNormal ) > 0 )
		{
			if ( !(pPortal->nFlags & FVIS_PORTAL_FLAG_MIRROR) && pVolume->nActiveAdjacentSteps 
				&& (!FVis_bInRenderTarget || FVis_bAllowLiquid_RenderTarget) )
			{
				// Submit this volume to the active list
				_AL_AddVolume( pAdjVolume, pPortal->anIdxInVolume[nAdjVolumePortalIdx], pVolume->nActiveAdjacentSteps );
			}
			continue;
		}
#if 1
		s32 ii;
		// If none of this portal's verts lie on the other side of the portal we're looking
		// through, then we cannot see through this portal in this iteration of the visibility
		// traversal, even if the portal is facing the camera.
		for ( ii = 0; ii < pPortal->nVertCount; ii++ )
		{
			vDiff = pPortal->avVertices[ii] - vViewPortalCenter;
			if ( vDiff.Dot( vViewPortalNormal ) > 0.f )
			{
				break;
			}
		}
		
		if ( ii == pPortal->nVertCount )
		{
			continue;
		}
#endif

		if ( _ActiveOccluderCount && _Occlusion_PortalIsOccluded( pPortal ) )
		{
			continue;
		}
		
		#if FANG_DEBUG_BUILD && _DRAW_VIS_FRUSTRUMS
			_nPortalConsidered = pVolume->paPortalIndices[i];
		#endif

		// Check for visibility and clipping
		if ( !_ClipViewportToPortal( pPortal->avVertices, nVolumePortalIdx ) )
		{
			// Portal is not visible so go to the next one
			if ( !(pPortal->nFlags & FVIS_PORTAL_FLAG_MIRROR) && pVolume->nActiveAdjacentSteps 
				&& (!FVis_bInRenderTarget || FVis_bAllowLiquid_RenderTarget) )
			{
				// Submit this volume to the active list
				_AL_AddVolume( pAdjVolume, pPortal->anIdxInVolume[nAdjVolumePortalIdx], pVolume->nActiveAdjacentSteps );
			}
			continue;
		}
#if 0
		// To prevent looping, we only allow each portal to be visited 3 times
		if ( _GetTimesVisitedPortal( pVolume->paPortalIndices[i] ) == 3 )
		{
			// Return the stack state
			FVis_VPNormalStack.PopBase();
			continue;
		}
		_IncrementTimesVisitedPortal( pVolume->paPortalIndices[i] );
#endif

		#if _PORTAL_MIRRORS_ENABLED
			// If this portal is a mirror, we need to set the flag so that
			// anything seen through it will use the mirror matrix
			if ( pPortal->nFlags & FVIS_PORTAL_FLAG_MIRROR )
			{
				// We currently don't support multiple reflections
				if ( _nUseMirror )
				{
					continue;
				}
				if ( _nMirrorsInViewCount + 1 > _nMaxMirrorsInView )
				{
					continue;
				}
				
				_nUseMirror = 1 + (_nUseMirror << 1);
				
				// Generate the portal orientation
				CFMtx43A mtxPortal;
				f32 fAngleY = fmath_Atan( vPortalNormal.x, vPortalNormal.z );
				f32 fAngleX = -fmath_Atan( vPortalNormal.y, vPortalNormal.MagXZ() );
				mtxPortal.Identity();
				mtxPortal.RotateY( fAngleY );
				mtxPortal.RotateX( fAngleX );
				
				// Put portal position in center (Wouldn't matter if we weren't doing effects
				mtxPortal.m_vPos.Set( pPortal->avVertices[0] );
				mtxPortal.m_vPos.v3 += pPortal->avVertices[1];
				mtxPortal.m_vPos.v3 += pPortal->avVertices[2];
				mtxPortal.m_vPos.v3 += pPortal->avVertices[3];
				mtxPortal.m_vPos.Mul( 0.25f );
				_vMirrorNormal.Set( vPortalNormal );
				_vMirrorPosition.Set( mtxPortal.m_vPos );
				
				// Generate the mirror matrix in the next mirror matrix buffer slot
				_pMirrorMtx = &_paMirrorViewMtx[_nMirrorsInViewCount++];
				_pMirrorMtx->Set( mtxPortal );
				
				CFMtx43A mtxReflection( 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f );

				// Cheesy warping of the mirror
				if ( i == 0 )
				{			
					static f32 __fCurrent;
					static f32 __fInc;
					static f32 __fMag;
					static u32 __nTimer = 0;
					
					if ( __nTimer == 0 )
					{
						__fCurrent = FMATH_HALF_PI;
						__nTimer = 1000;
						__fInc = 0.15f;
						__fMag = 0.11f;
					}
					__nTimer--;
			
					mtxReflection.m_vRight.x += __fMag * fmath_Sin( __fCurrent - FMATH_HALF_PI );
					
					f32 fLast = __fCurrent;
					__fCurrent += __fInc;
					if ( (fLast - FMATH_HALF_PI) * (__fCurrent - FMATH_HALF_PI) < 0.f )
					{
						__fMag *= 0.8f;
					}
					if ( __fCurrent > FMATH_PI )
					{					
						__fInc = -__fInc;
						__fCurrent = FMATH_PI;
					}
					else if ( __fCurrent < 0 )
					{					
						__fInc = -(__fInc);// * 0.80f);
						__fCurrent = 0;
					}
				}

				_pMirrorMtx->Mul( mtxReflection );
				_pMirrorMtx->Mul( mtxPortal.AffineInvert( FALSE ) );
			}
		#endif //_PORTAL_MIRRORS_ENABLED

		// Portal is visible and open, so we should add the volume
		_DL_WalkVisibility( &FVis_pVisData->paVolumes[pPortal->anAdjacentVolume[nAdjVolumePortalIdx]], pPortal->anIdxInVolume[nAdjVolumePortalIdx] );

		#if _PORTAL_MIRRORS_ENABLED
			// If this was a mirror, remove the flag
			if ( pPortal->nFlags & FVIS_PORTAL_FLAG_MIRROR )
			{
				_nUseMirror >>= 1;
			}
		#endif // _PORTAL_MIRRORS_ENABLED

		// Return the stack state
		FVis_VPNormalStack.PopBase();
	}
}


//
//	This adds a portal to the active list, and recurses through adjacent volumes
//	adding them to the active list until nStepsLeft == 0.
//
static void _AL_AddVolume( FVisVolume_t *pVolume, u32 nPortalAddedFrom, s32 nStepsLeft )
{
	s32 i;
	
	// Set the Volume Flags
	FVis_anVolumeFlags[pVolume->nVolumeID] |= FVIS_VOLUME_IN_ACTIVE_LIST;
	
	if ( pVolume->pWorldGeo )
	{
		pVolume->pWorldGeo->CacheMeshData( 1 );
	}

	nStepsLeft -= pVolume->nActiveStepDecrement;
	if ( nStepsLeft <= 0 )
	{
		return;
	}
	
	// Check this volume's portals to determine visible adjacencies
	for ( i = 0; i < pVolume->nPortalCount; i++ )
	{
		// Skip the portal that we are looking into this room from
		if ( i == (s32)nPortalAddedFrom )
		{
			continue;
		}
		
		FVisPortal_t *pPortal = &FVis_pVisData->paPortals[pVolume->paPortalIndices[i]];
		
		// Skip if this is not a connecting portal
		if ( pPortal->nFlags & (FVIS_PORTAL_FLAG_MIRROR | FVIS_PORTAL_FLAG_ANTIPORTAL) )
		{
			continue;
		}
		
		// Determine our index in the portal connectivity description
		u32 nVolumePortalIdx, nAdjVolumePortalIdx;
		if ( pPortal->anAdjacentVolume[1] != pVolume->nVolumeID )
		{
			nVolumePortalIdx = 0;
			nAdjVolumePortalIdx = 1;
		}
		else
		{
			nAdjVolumePortalIdx = 0;
			nVolumePortalIdx = 1;
		}

		// Add adjacent portals
		_AL_AddVolume( &FVis_pVisData->paVolumes[pPortal->anAdjacentVolume[nAdjVolumePortalIdx]], pPortal->anIdxInVolume[nAdjVolumePortalIdx], nStepsLeft );
	}
}

//
//	Returns TRUE if portal was within the viewport or false if not
//
static BOOL _ClipViewportToPortal( const CFVec3 *pPortalVerts, u32 nVolumePortalIdx )
{
	u32 i;
	CFVec3 vDiff;

	FASSERT( pPortalVerts );
	
	// Check against the camera position and facing
	for ( i = 0; i < FVIS_MAX_PORTAL_VERTS; i++ )
	{
/*	
		if ( FVis_VPNormalStack.PointIsVisible( &pPortalVerts[i] ) )
		{
			break;
		}
*/
		vDiff = pPortalVerts[i] - _vCameraPos.v3;
		if ( vDiff.Dot( _vNearPlaneNormal ) > 0 )
		{
			break;
		}
	}
	
	// If we cycled through all verts and they were behind the camera, bail
	if ( i == FVIS_MAX_PORTAL_VERTS )
	{
		return FALSE;
	}
			
	// Check against the far plane (we don't clip it against the far plane)
	for ( i = 0; i < FVIS_MAX_PORTAL_VERTS; i++ )
	{
		vDiff = pPortalVerts[i] - _vFarPlanePoint;
		if ( vDiff.Dot( _vFarPlaneNormal ) > 0 )
		{
			break;
		}
	}
	
	// If we cycled through all verts and they were beyond the far plane, bail
	if ( i == FVIS_MAX_PORTAL_VERTS )
	{
		return FALSE;
	}
	
	// Setup the vertex clip buffer in preparation for clipping		
	u32 nBufferIdx = 0;
	u32 nVertCount = FVIS_MAX_PORTAL_VERTS;
	for ( i = 0; i < FVIS_MAX_PORTAL_VERTS; i++ )
	{
		_avClipBuffer[nBufferIdx][i].Set( pPortalVerts[i] );
	}
	
	CFVec3A *paNormals = FVis_VPNormalStack.GetCurrentBase();
	u32 nNormalCount = FVis_VPNormalStack.GetCurrentCount();
	
	FASSERT( paNormals );
	
	// Clip the portal against the viewport side planes
	for ( i = 0; i < nNormalCount; i++ )
	{
		if ( !_ClipVertsToPlane( &nBufferIdx, &nVertCount, &paNormals[i] ) )
		{
			// All of the verts ended up on the negative side
			// of the plane so they cannot be visible.
			return FALSE;
		}
	}
	
	FASSERT( nVertCount > 0 && nVertCount <= _AV_CLIP_BUFFER_SIZE );
	FASSERT( nBufferIdx == 0 || nBufferIdx == 1 );

	// Push a new base onto the viewport plane normal stack.
	FVis_VPNormalStack.PushBase();
	
#if FANG_DEBUG_BUILD && _DRAW_VIS_FRUSTRUMS
	_DrawViewportExtents( &_vCameraPos, _avClipBuffer[nBufferIdx], nVertCount );
#endif

	// Generate the new viewport planes based on the clipped portal.
	//	Note that we need to handle the creation of these planes
	//	differently if we are looking from the backside of the
	//	portal because the winding order of the verts changes.  We
	//	have to do this to ensure that the viewport normals always
	//	face into the viewspace.
	CFVec3A *paVerts = _avClipBuffer[nBufferIdx];
	FASSERT( paNormals );
	if ( nVolumePortalIdx == 0 )
	{
		// If we're dealing with the portal as seen from volume 0, our
		// winding order for the verts is clockwise
		u32 iPriorVert = nVertCount - 1;
		CFVec3A vDiff1, vDiff2;
		for ( i = 0; i < nVertCount; i++ )
		{
			vDiff1.Sub( paVerts[i], paVerts[iPriorVert] );
			vDiff2.Sub( paVerts[i], _vCameraPos );
			if ( vDiff1.MagSq() < 0.01f || vDiff2.MagSq() < 0.01f )
			{
				iPriorVert = i;
				continue;
			}
			FVis_VPNormalStack.AddNormal( vDiff1.Cross( vDiff2 ).Unitize() );
			iPriorVert = i;
		}
	}
	else
	{
		// If we're dealing with the portal as seen from volume 1, our
		// winding order for the verts is counter-clockwise
		u32 iPriorVert = nVertCount - 1;
		CFVec3A vDiff1, vDiff2;
		for ( i = 0; i < nVertCount; i++ )
		{
			vDiff1.Sub( paVerts[iPriorVert], paVerts[i] );
			vDiff2.Sub( paVerts[iPriorVert], _vCameraPos );
			if ( vDiff1.MagSq() < 0.01f || vDiff2.MagSq() < 0.01f )
			{
				iPriorVert = i;
				continue;
			}
			FVis_VPNormalStack.AddNormal( vDiff1.Cross( vDiff2 ).Unitize() );
			iPriorVert = i;
		}
	}
	
	return TRUE;	
}


//
//	Returns FALSE if the verts are all on the negative side of the plane. If
//	all of the points are on the positive side, returns TRUE without clipping
//	any of the verts or changing the vert count or buffer.  If the verts are
//	seperated by the plane, the edges created by the verts are clipped to the
//	positive side and the new verts are stored in the next buffer.  Vert counts 
//	and buffer index are set appropriately.
//
static BOOL _ClipVertsToPlane( u32 *pnBufferIdx, u32 *pnVertCount, const CFVec3A *pPlaneNormal )
{
	// TODO: If we ever have more than 32 verts, we need to just take the 32 longest edges
	FASSERT( pnVertCount && pnBufferIdx && pPlaneNormal );
	FASSERT( *pnVertCount < 32 );

	f32 afDistance[_AV_CLIP_BUFFER_SIZE];

	// Get pointers to the vert buffer
	CFVec3A *paOldVerts = _avClipBuffer[*pnBufferIdx];
	CFVec3A vDiff;

	// Determine the distance to the plane
	u32 i, nInsideBits = 0;
	for ( i = 0; i < *pnVertCount; i++ )
	{
		vDiff.Sub( paOldVerts[i], _vCameraPos );
		afDistance[i] = vDiff.Dot( *pPlaneNormal );
		
		if ( afDistance[i] >= 0 )
		{
			nInsideBits |= (1 << i);
		}
	}

	// If all of the points are on the negative side of the 
	// plane, no clipping is necessary and we return FALSE.
	if ( nInsideBits == 0 )
	{
		return FALSE;
	}

	// If all of the points are on the positive side of the 
	// plane, no clipping is necessary and we return TRUE.
	if ( nInsideBits == (u32)(1 << (*pnVertCount)) - 1 )
	{
		return TRUE;
	}

	// Swap buffers
	(*pnBufferIdx) = (*pnBufferIdx + 1) & 0x01;
	CFVec3A *paNewVerts = _avClipBuffer[*pnBufferIdx];

	// Otherwise, the plane separates the points and we need
	// to perform clipping.  Edges created by the verts will
	// be clipped to the positive side of the plane.
	u32 nNewVertCount = 0;
	u32 iPriorVert = *pnVertCount - 1;
	for ( i = 0; i < *pnVertCount; i++ )
	{
		if ( afDistance[i] >= 0 )
		{
			if ( afDistance[iPriorVert] >= 0 )
			{
				// Edge is entirely on positive side so we don't need to clip
				// Add the current vert.
				paNewVerts[nNewVertCount++].Set( paOldVerts[i] );
			}
			else
			{
				// We're going from positive to negative side, so we need to clip.
				// We use an approximate division, which will hopefully be good enough
				paNewVerts[nNewVertCount++].Lerp( fmath_Div( afDistance[i], afDistance[i] - afDistance[iPriorVert] ), paOldVerts[i], paOldVerts[iPriorVert] );
				
				// Add in the current vert, too, since it is on the positive side
				paNewVerts[nNewVertCount++].Set( paOldVerts[i] );
				
			}
		}
		else if (afDistance[iPriorVert] >= 0 )
		{
			// We're going from negative to positive side, so we need to clip.
			// We use an approximate division, which will hopefully be good enough
			paNewVerts[nNewVertCount++].Lerp( fmath_Div( afDistance[iPriorVert], afDistance[iPriorVert] - afDistance[i] ), paOldVerts[iPriorVert], paOldVerts[i] );
		}

		// If we fail all of the above tests, the edge is entirely on the 
		// negative side, so we don't need to clip and don't want to use 
		// the current vert.

		iPriorVert = i;
	}

	// Set the new vert count	
	*pnVertCount = nNewVertCount;

	return TRUE;
}	
	

#if FANG_DEBUG_BUILD && _DRAW_VIS_FRUSTRUMS
//
//
//
static void _DrawViewportExtents( CFVec3A *pCamPos, CFVec3A *paVerts, u32 nPointCount )
{
	u32 i;
	FDrawVtx_t vFDrawVerts[64];
	
	// Setup fdraw for rendering of the volumes
	frenderer_Push( FRENDERER_DRAW, NULL );

	// Set up an identity Xfm to push
	CFXfm xfmIdentity;
	xfmIdentity.Identity();
	xfmIdentity.PushModel();

	// Set the states we need
	fdraw_Depth_EnableWriting( TRUE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_SRC );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_SetCullDir( FDRAW_CULLDIR_NONE );
	
	f32 fRed = (f32)(_nPortalConsidered & 0xf) * 0.0624f;
	f32 fGreen = (f32)((_nPortalConsidered + 4) & 0xf) * 0.0624f;
	
	for ( i = 0; i < nPointCount; i++ )
	{
		vFDrawVerts[i*2].Pos_MS = pCamPos->v3;
		vFDrawVerts[i*2].ColorRGBA.Set( fRed, fGreen, 0.1f, 1.f );
		vFDrawVerts[(i*2)+1].Pos_MS = paVerts[i].v3;
		vFDrawVerts[(i*2)+1].ColorRGBA.Set( fRed, fGreen, 0.1f, 1.f );
	}
	
	fdraw_PrimList( FDRAW_PRIMTYPE_LINELIST, vFDrawVerts, nPointCount * 2 );

	for ( i = 0; i < nPointCount; i++ )
	{
		vFDrawVerts[i].Pos_MS = paVerts[i].v3;
		vFDrawVerts[i].ColorRGBA.Set( fRed, fGreen, 0.1f, 1.f );
	}
	vFDrawVerts[i].Pos_MS = paVerts[0].v3;
	vFDrawVerts[i].ColorRGBA.Set( fRed, fGreen, 0.1f, 1.f );
	
	fdraw_PrimList( FDRAW_PRIMTYPE_LINESTRIP, vFDrawVerts, nPointCount + 1 );

	xfmIdentity.PopModel();	
	frenderer_Pop();
}	
#endif

void fvis_DrawLiquidVolumes()
{
#if !FANG_PLATFORM_WIN
	BOOL bFogEnable = fsh_Fog_IsEnabled();
	fsh_Fog_Enable(FALSE);
	
	#if FANG_PLATFORM_GC
		fgc_DisableChannels();
	#endif

	if (!FVis_bInRenderTarget || FVis_bAllowLiquid_RenderTarget)
	{
		LiquidSystem.Work();
		LiquidSystem.Render();
	}

	fsh_Fog_Enable(bFogEnable);
#endif
}


//
//
//
FVisPortal_t* fvis_GetNearestPortal( const CFVec3A *pvPos_WS, f32 *pDistance )
{
	u32 i, ii;
	f32 fClosestSq = FMATH_MAX_FLOAT;
	FVisPortal_t *pClosest = NULL;
	
	for ( i = 0; i < FVis_pVisData->nPortalCount; i++ )
	{
		FVisPortal_t *pPortal = &FVis_pVisData->paPortals[i];
		
		CFVec3A vAveragePos( 0.f, 0.f, 0.f );
		for ( ii = 0; ii < FVis_pVisData->paPortals[i].nVertCount; ii++ )
		{
			vAveragePos.v3 += pPortal->avVertices[ii];
		}
		
		vAveragePos.Mul( fmath_Inv( FVis_pVisData->paPortals[i].nVertCount ) );
		
		CFVec3A vDiff;
		vDiff.Sub( vAveragePos, *pvPos_WS );
		f32 fDistSq = vDiff.MagSq();
		if ( fDistSq < fClosestSq )
		{
			fClosestSq = fDistSq;
			pClosest = pPortal;
		}
	}
	
	if ( pDistance )
	{
		(*pDistance) = fmath_Sqrt( fClosestSq );
	}
	
	return pClosest;
}


#if _ENABLE_VOLUME_DRAWING
//
//
//
void fvis_CalculateVolumePlanes( FVisVolume_t *pVolume, FVisPlaneCallback_t *pCallBack )
{
	u32 i, ii, iii, nVert, iTest, nCIdx;
	CFVec3A vResult, vTemp;
	CFVec3A vDiff;
	
	for ( nCIdx = 0; nCIdx < pVolume->nCellCount; nCIdx++ )
	{
		FVisCell_t *pCell = &FVis_pVisData->paCells[pVolume->nCellFirstIdx + nCIdx];

		if ( pCell->nPlaneCount < 3 || pCell->nPlaneCount > FCONVHULL_MAX_PLANES )
		{
			continue;
		}
			
		fang_MemSet( FConvHull_PlaneBuffer, 0, sizeof( FConvHull_Plane_t ) * pCell->nPlaneCount );
		FConvHull_nPointBufferCount = 0;
	
		FVisPlane_t *pPlanes = pCell->paBoundingPlanes;
		
		// Set the plane normals
		for ( i = 0; i < pCell->nPlaneCount; i++ )
		{
			FConvHull_PlaneBuffer[i].vNormal.Set( pPlanes[i].vNormal );
		}
		
		for ( i = 0; i < (u32)(pCell->nPlaneCount) - 2; i++ )
		{
			for ( ii = i + 1; ii < (u32)(pCell->nPlaneCount - 1); ii++ )
			{
				for ( iii = ii + 1; iii < pCell->nPlaneCount; iii++ )
				{
					// Calculate the determinant
					f32 fDet = vTemp.Cross( pPlanes[i].vNormal, pPlanes[ii].vNormal ).Dot( pPlanes[iii].vNormal );
					if ( fDet > -0.001f && fDet < 0.001f )
					{
						continue;
					}
						
					vResult.Cross( pPlanes[ii].vNormal, pPlanes[iii].vNormal ).Mul( pPlanes[i].vPoint.Dot( pPlanes[i].vNormal ) );
					vTemp.Cross( pPlanes[iii].vNormal, pPlanes[i].vNormal ).Mul( pPlanes[ii].vPoint.Dot( pPlanes[ii].vNormal ) );
					vResult.Add( vTemp );
					vTemp.Cross( pPlanes[i].vNormal, pPlanes[ii].vNormal ).Mul( pPlanes[iii].vPoint.Dot( pPlanes[iii].vNormal ) );
					vResult.Add( vTemp ).Mul( 1.f / fDet );
					
					// Make sure this point is contained by the other planes
					for ( iTest = 0; iTest < pCell->nPlaneCount; iTest++ )
					{
						// Don't test the intersecting planes
						if ( iTest == i || iTest == ii || iTest == iii )
						{
							continue;
						}
							
						vDiff.Sub( vResult, pPlanes[iTest].vPoint );
						if ( vDiff.Dot( pPlanes[iTest].vNormal ) < 0 )
						{
							break;
						}
					}
					
					// If one of the planes failed to contain the point, don't add it
					if ( iTest != pCell->nPlaneCount )
					{
						continue;
					}
						
					// Verify that we don't already have this point
					for ( nVert = 0; nVert < FConvHull_nPointBufferCount; nVert++ )
					{
						vDiff.Sub( FConvHull_vPointBuffer[nVert], vResult );
						f32 fTest = vDiff.Mag();
						if ( fTest < 0.001f )
						{
							break;
						}
					}
					
					// Add this point to the list, if we need to
					if ( nVert == FConvHull_nPointBufferCount )
					{
						FASSERT( FConvHull_nPointBufferCount < FCONVHULL_MAX_PLANES * FCONVHULL_MAX_PLANE_POINTS );
						FConvHull_vPointBuffer[FConvHull_nPointBufferCount++].Set( vResult );
					}

					FASSERT( FConvHull_PlaneBuffer[i].nPointCount < FCONVHULL_MAX_PLANE_POINTS );
					FASSERT( FConvHull_PlaneBuffer[ii].nPointCount < FCONVHULL_MAX_PLANE_POINTS );
					FASSERT( FConvHull_PlaneBuffer[iii].nPointCount < FCONVHULL_MAX_PLANE_POINTS );
					FConvHull_PlaneBuffer[i].AddIndex( nVert );
					FConvHull_PlaneBuffer[ii].AddIndex( nVert );
					FConvHull_PlaneBuffer[iii].AddIndex( nVert );
				}
			}
		}
		
		CFVec3A vRandomDiff, vCross;
		for ( i = 0; i < pCell->nPlaneCount; i++ )
		{
			f32 fSortVal[FCONVHULL_MAX_PLANE_POINTS];

			if ( FConvHull_PlaneBuffer[i].nPointCount < 3 )
			{
				continue;
			}

			u16 *pPoints = FConvHull_PlaneBuffer[i].nPointIdx;

			vRandomDiff.Sub( FConvHull_vPointBuffer[ pPoints[1] ], FConvHull_vPointBuffer[ pPoints[0] ] );
			vRandomDiff.Unitize();
			vCross.Cross( vRandomDiff, pPlanes[i].vNormal );
			
			fSortVal[0] = -1.570796f;
			fSortVal[1] = 1.570796f;
			
			for ( ii = 2; ii < FConvHull_PlaneBuffer[i].nPointCount; ii++ )
			{
				vDiff.Sub( FConvHull_vPointBuffer[ pPoints[ii] ], FConvHull_vPointBuffer[ pPoints[0] ] );
				vDiff.Unitize();
				f32 x = vCross.Dot( vDiff );
				f32 y = vRandomDiff.Dot( vDiff );
				fSortVal[ii] = fmath_Atan( y, x );
			}
			
			for ( ii = 0; ii < FConvHull_PlaneBuffer[i].nPointCount - 1; ii++ )
			{
				for ( iii = ii + 1; iii < FConvHull_PlaneBuffer[i].nPointCount; iii++ )
				{
					if ( fSortVal[iii] > fSortVal[ii] )
					{
						continue;
					}
					
					f32 fTemp = fSortVal[ii];
					u16 nTemp = pPoints[ii];
					fSortVal[ii] = fSortVal[iii];
					pPoints[ii] = pPoints[iii];
					fSortVal[iii] = fTemp;
					pPoints[iii] = nTemp;
				}
			}
		}
		
		if ( !pCallBack( pCell->nPlaneCount ) )
		{
			return;
		}
	}
}

u32 _nIdxOfVolDrawing;
//
//
//
static BOOL _VolumePlaneCallback( u32 nPlanes )
{
	static CFVec3 __vRandomColors[8] = 
	{ CFVec3( 0, 174, 239 ),CFVec3( 0, 166, 81 ),	CFVec3( 166, 124, 82 ),	CFVec3( 171, 0, 193),
	  CFVec3( 0, 200, 0 ),	CFVec3( 128, 117, 255 ),CFVec3( 0, 0, 200 ),	CFVec3( 200, 200, 200) };
	
	u32 i, ii;
	f32 fR, fG, fB, fA;
	f32 fLR, fLG, fLB, fLA = 1.f;

	fA = 0.2f;

	fLR = fR = __vRandomColors[_nIdxOfVolDrawing & 7].x * (1.f / 255.f);
	fLG = fG = __vRandomColors[_nIdxOfVolDrawing & 7].y * (1.f / 255.f);
	fLB = fB = __vRandomColors[_nIdxOfVolDrawing & 7].z * (1.f / 255.f);
	
	fdraw_Depth_EnableWriting( FALSE );

	FDrawVtx_t vVolVerts[45];
	CFVec3A vFakeLight( 0.57735f, -0.57735f, 0.57735f );
	for ( i = 0; i < nPlanes; i++ )
	{
		if ( FConvHull_PlaneBuffer[i].nPointCount < 3 )
		{
			continue;
		}
		
		f32 fDot = vFakeLight.Dot( FConvHull_PlaneBuffer[i].vNormal );
		fDot = fmath_Div( fDot, 4.f) + 0.75f;
		CFColorRGBA FaceColor( fR * fDot, fG * fDot, fB * fDot, fA );

		u32 nVertCount = 0;
		s32 nLastVert = 1;
		for ( ii = 2; ii < FConvHull_PlaneBuffer[i].nPointCount; ii++ )
		{
			if ( nVertCount + 3 > 45 )
			{
				break;
			}
			
			vVolVerts[nVertCount].Pos_MS = FConvHull_vPointBuffer[ FConvHull_PlaneBuffer[i].nPointIdx[0] ].v3;
			vVolVerts[nVertCount].ColorRGBA.Set( fR, fG, fB, fA );
			nVertCount++;
			
			vVolVerts[nVertCount].Pos_MS = FConvHull_vPointBuffer[ FConvHull_PlaneBuffer[i].nPointIdx[nLastVert] ].v3;
			vVolVerts[nVertCount].ColorRGBA.Set( fR, fG, fB, fA );
			nVertCount++;
			
			vVolVerts[nVertCount].Pos_MS = FConvHull_vPointBuffer[ FConvHull_PlaneBuffer[i].nPointIdx[ii] ].v3;
			vVolVerts[nVertCount].ColorRGBA.Set( fR, fG, fB, fA );
			nVertCount++;
			
			nLastVert = ii;
		}
		
		fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, vVolVerts, nVertCount );
	}
	
	fdraw_Depth_EnableWriting( TRUE );
	for ( i = 0; i < nPlanes; i++ )
	{
		if ( FConvHull_PlaneBuffer[i].nPointCount < 2 )
		{
			continue;
		}
		
		for ( ii = 0; ii < FConvHull_PlaneBuffer[i].nPointCount; ii++ )
		{
			vVolVerts[ii].Pos_MS = FConvHull_vPointBuffer[ FConvHull_PlaneBuffer[i].nPointIdx[ii] ].v3;
			vVolVerts[ii].ColorRGBA.Set( fLR, fLG, fLB, fLA );
		}
		
		fdraw_PrimList( FDRAW_PRIMTYPE_LINESTRIP, vVolVerts, ii );
	}
	
	return TRUE;
}


//
//
//
static void _DrawVolumePlanes( void )
{
	static s32 nPortalFlashTimer = 10;
	static BOOL bPortalOn = FALSE;
	
#if 1
	nPortalFlashTimer--;
	if ( nPortalFlashTimer < 0 )
	{
		bPortalOn = !bPortalOn;
		if ( !bPortalOn )
		{
			nPortalFlashTimer = 30;
		}
		else
		{
			nPortalFlashTimer = 10;
		}
	}
#endif
	
	// Setup fdraw for rendering of the volumes
	frenderer_Push( FRENDERER_DRAW, NULL );

	// Set up an identity Xfm to push
	CFXfm xfmIdentity;
	xfmIdentity.Identity();
	xfmIdentity.PushModel();

	// Set the states we need
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_SetCullDir( FDRAW_CULLDIR_NONE );

	u32 i, ii, iii;
	for ( i = 0; i < FVis_pVisData->nVolumeCount; i++ )
	{
		_nIdxOfVolDrawing = i;
		
		if ( !(FVis_anVolumeFlags[_nIdxOfVolDrawing] & FVIS_VOLUME_IN_VISIBLE_LIST) )
		{
			continue;
		}

		// Make the call to generate the planes
		fvis_CalculateVolumePlanes( &FVis_pVisData->paVolumes[i], _VolumePlaneCallback );

		// Draw cell bounding spheres
/*
		for ( ii = 0; ii < FVis_pVisData->paVolumes[i].nCellCount; ii++ )
		{
			FVisCell_t *pCell = &FVis_pVisData->paCells[ FVis_pVisData->paVolumes[i].nCellFirstIdx + ii ];
			fdraw_FacetedWireSphere( &pCell->spBoundingWS.m_Pos, pCell->spBoundingWS.m_fRadius );
		}
*/		
		// Check this volume's portals to determine visible adjacencies
		if ( bPortalOn )
		{
			fdraw_Depth_EnableWriting( FALSE );
			
			for ( ii = 0; ii < FVis_pVisData->paVolumes[i].nPortalCount; ii++ )
			{
				FVisPortal_t *pPortal = &FVis_pVisData->paPortals[ FVis_pVisData->paVolumes[i].paPortalIndices[ii] ];
				
				FDrawVtx_t PortalVerts[(FVIS_MAX_PORTAL_VERTS - 2) * 3];
				
				CFColorRGBA Color;
				if ( pPortal->nFlags & FVIS_PORTAL_FLAG_AUTOPORTAL )
				{
					Color.Set( 1.f, 0.f, 0.f, 0.2f );
				}
				else
				{
					Color.Set( 1.f, 1.f, 0.f, 0.2f );
				}

				u32 nLastVert = 1, nVertCounter = 0;
				for ( iii = 2; iii < FVIS_MAX_PORTAL_VERTS; iii++ )
				{
					PortalVerts[nVertCounter].Pos_MS = pPortal->avVertices[0];
					PortalVerts[nVertCounter].ColorRGBA.Set( Color );
					nVertCounter++;

					PortalVerts[nVertCounter].Pos_MS = pPortal->avVertices[nLastVert];
					PortalVerts[nVertCounter].ColorRGBA.Set( Color );
					nVertCounter++;

					PortalVerts[nVertCounter].Pos_MS = pPortal->avVertices[iii];
					PortalVerts[nVertCounter].ColorRGBA.Set( Color );
					nVertCounter++;

					nLastVert = iii;
				}
				
				fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, PortalVerts, nVertCounter );
			}
		}
	}
	
	// Pop the model matrix	
	xfmIdentity.PopModel();
	
	// Pop the render
	frenderer_Pop();
}
#endif _ENABLE_VOLUME_DRAWING


//
//	In-place fixup of the portal data.
//
//		return:
//			TRUE - no error condition encountered
//			FALSE - error condition encountered
//
BOOL fvis_FixupPortalData( FVisData_t *pPortalData )
{
	FASSERT( pPortalData );
	
	// Macro that fixes up pointers IFF the pointer is non-NULL
	#define __FIXUP_POINTER( pointer, type )	if ( pointer ) pointer = (type *)((u32)pPortalData + (u32)pointer);
	
	u32 i;
	
	// Fixup the cell tree
	__FIXUP_POINTER( pPortalData->CellTree.paNodes, FVisCellTreeNode_t );
	
	// Fixup the portals
	__FIXUP_POINTER( pPortalData->paPortals, FVisPortal_t );

	// Fixup the volumes
	__FIXUP_POINTER( pPortalData->paVolumes, FVisVolume_t );
	for ( i = 0; i < pPortalData->nVolumeCount; i++ )
	{
		FVisVolume_t *pVolume = &pPortalData->paVolumes[i];
		
		__FIXUP_POINTER( pVolume->paPortalIndices, u16 );
	}

	if ( pPortalData->nPortalCount )
	{
		for ( i = 0; i < pPortalData->nPortalCount; i++ )
		{
			FVisPortal_t *pPortal = &pPortalData->paPortals[i];
			pPortal->fBoundingRadiusSq = pPortal->spBoundingWS.m_fRadius * pPortal->spBoundingWS.m_fRadius;
			if ( fmath_Abs(pPortal->vNormal.x) > fmath_Abs(pPortal->vNormal.y) && fmath_Abs(pPortal->vNormal.x) > fmath_Abs(pPortal->vNormal.z) )
			{
				pPortal->nFlags |= FVIS_PORTAL_FLAG_NORM_X_LARGEST;
			}
			else if  ( fmath_Abs(pPortal->vNormal.y) > fmath_Abs(pPortal->vNormal.x) &&fmath_Abs( pPortal->vNormal.y) > fmath_Abs(pPortal->vNormal.z) )
			{
				pPortal->nFlags |= FVIS_PORTAL_FLAG_NORM_Y_LARGEST;
			}
			else
			{
				pPortal->nFlags |= FVIS_PORTAL_FLAG_NORM_Z_LARGEST;
			}
		}
	}
			
#if _MIRROR_HACK
	if ( pPortalData->nPortalCount )
	{
		for ( i = 0; i < pPortalData->nPortalCount; i++ )
		{
			FVisPortal_t *pPortal = &pPortalData->paPortals[i];
			
			pPortal->nFlags = FVIS_PORTAL_FLAG_MIRROR;
			if ( pPortal->anAdjacentVolume[0] != 0 )
			{
				pPortal->vNormal = -pPortal->vNormal;
				CFVec3 vTemp;
				vTemp = pPortal->avVertices[1];
				pPortal->avVertices[1] = pPortal->avVertices[3];
				pPortal->avVertices[3] = vTemp;
			}
			pPortal->anAdjacentVolume[0] = 0;
			pPortal->anAdjacentVolume[1] = 0;
			pPortal->anIdxInVolume[0] = (u8)i;
			pPortal->anIdxInVolume[1] = (u8)i;
		}
	}
#endif	

	// Fixup the cells
	__FIXUP_POINTER( pPortalData->paCells, FVisCell_t );
	for ( i = 0; i < pPortalData->nCellCount; i++ )
	{
		FVisCell_t *pCell = &pPortalData->paCells[i];
		
		__FIXUP_POINTER( pCell->paBoundingPlanes, FVisPlane_t );
	}

	// Fixup the directional lights
	__FIXUP_POINTER( pPortalData->paLights, FLightInit_t );

	#undef __FIXUP_POINTER

	return TRUE;	
}


//
//	Given a pointer to an FVisPortalData_t structure, this function packs
//	the data into contiguous memory space in preparation for writing
//	to file.
//
//		Return value:
//			0 = unsuccessful attempt to pack data.
//			non zero = sizeof the packed data.
//
//	It is the responsibility of the calling function to free the packed
//	data after it is finished using it.
//
//
u32 fvis_PackPortalData( FVisData_t *pPortalData, BOOL bChangeEndian, void **pPackedData, u32 *pnDataSize )
{
	u32 i, ii;
	*pnDataSize = 0;
	*pPackedData = NULL;
	
	if ( !pPortalData )
	{
		FASSERT_NOW;
		return FVIS_PACK_NO_PORTAL_DATA;
	}

	if ( pPortalData->nVolumeCount > FVIS_MAX_VOLUME_COUNT )
	{
		FASSERT_NOW;
		return FVIS_PACK_EXCEEDED_MAX_VOLUMES;
	}

	if ( pPortalData->nPortalCount > FVIS_MAX_PORTAL_COUNT )
	{
		FASSERT_NOW;
		return FVIS_PACK_EXCEEDED_MAX_PORTALS;
	}

	////////////////////////////////////////////////////////////
	// MOVE ANY LIGHT MAP LIGHTS TO THE END OF THE LIGHT DATA
	////////////////////////////////////////////////////////////

	FLightInit_t TempLightInit;
	u32 nValidLightCount = 0;
	if ( pPortalData->nLightCount )
	{
		for ( i = 0; i < (u32)pPortalData->nLightCount; i++ )
		{
			if ( !(pPortalData->paLights[i].nFlags & FLIGHT_FLAG_ENGINE_LIGHT ) )
			{
				for ( ii = i + 1; ii < pPortalData->nLightCount; ii++ )
				{
					if ( pPortalData->paLights[ii].nFlags & FLIGHT_FLAG_ENGINE_LIGHT )
					{
						// Swap the two light inits
						fang_MemCopy( &TempLightInit, &pPortalData->paLights[i], sizeof( FLightInit_t ) );
						fang_MemCopy( &pPortalData->paLights[i],  &pPortalData->paLights[ii], sizeof( FLightInit_t ) );
						fang_MemCopy( &pPortalData->paLights[ii], &TempLightInit, sizeof( FLightInit_t ) );
						break;
					}
				}
				if ( ii == pPortalData->nLightCount )
				{
					// All the rest of the lights are light map lights, so we're done
					break;
				}
			}
		}
		nValidLightCount = i;
	}
	
	if ( nValidLightCount > FVIS_MAX_LIGHTS )
	{
		FASSERT_NOW;
		return FVIS_PACK_EXCEEDED_MAX_LIGHTS;
	}
	
	////////////////////////////////////////////////////////////
	// CALCULATE SIZE OF PORTAL DATA
	////////////////////////////////////////////////////////////
	
	*pnDataSize += sizeof( FVisData_t );
	
	// Calculate size of Cell Tree Data
	u32 ADDR_CellTreeNodeData = FMATH_BYTE_ALIGN_UP( *pnDataSize, 4 );
	*pnDataSize = ADDR_CellTreeNodeData + sizeof( FVisCellTreeNode_t ) * pPortalData->CellTree.nNodeCount;
	*pnDataSize = FMATH_BYTE_ALIGN_UP( *pnDataSize, 4 );

	// Calculate size of portal data
	u32 ADDR_PortalData = FMATH_BYTE_ALIGN_UP( *pnDataSize, 4 );
	*pnDataSize = ADDR_PortalData + sizeof( FVisPortal_t ) * pPortalData->nPortalCount;

	// Calculate size of Volume data
	u32 ADDR_VolumeData = FMATH_BYTE_ALIGN_UP( *pnDataSize, 4 );
	*pnDataSize = ADDR_VolumeData + sizeof( FVisVolume_t ) * pPortalData->nVolumeCount;
	for ( i = 0; i < pPortalData->nVolumeCount; i++ )
	{
		FVisVolume_t *pVolume = &pPortalData->paVolumes[i];
//		*pnDataSize += FMATH_BYTE_ALIGN_UP( sizeof( u16 ) * pVolume->nCellCount, 4 );
		*pnDataSize += FMATH_BYTE_ALIGN_UP( sizeof( u16 ) * pVolume->nPortalCount, 4 );
	}
	
	// Calculate size of Cell data
	u32 ADDR_CellData = FMATH_BYTE_ALIGN_UP( *pnDataSize, 4 );
	*pnDataSize = ADDR_CellData + sizeof( FVisCell_t ) * pPortalData->nCellCount;

	// Calculate size of Cell plane data
	u32 ADDR_CellPlaneData = FMATH_BYTE_ALIGN_UP( *pnDataSize, 16 );
	*pnDataSize = ADDR_CellPlaneData;
	for ( i = 0; i < pPortalData->nCellCount; i++ )
	{
		FVisCell_t *pCell = &pPortalData->paCells[i];
		*pnDataSize += FMATH_BYTE_ALIGN_UP( sizeof( FVisPlane_t ) * pCell->nPlaneCount, 16 );
	}
	
	// Calculate size of light data
	u32 ADDR_LightData = FMATH_BYTE_ALIGN_UP( *pnDataSize, 4 );
	*pnDataSize = ADDR_LightData + sizeof( FLightInit_t ) * nValidLightCount;
	
	////////////////////////////////////////////////////////////
	// ALLOCATE PACKED DATA MEMORY
	////////////////////////////////////////////////////////////
	
	// Allocate space to hold the resulting packed data
	*pnDataSize = FMATH_BYTE_ALIGN_UP( *pnDataSize, FCLASS_BYTE_ALIGN );
	*pPackedData = fang_MallocAndZero( *pnDataSize, 0 );
	if ( !pPackedData )
	{
		return FVIS_PACK_NO_MEMORY;
	}

	// Get a handy pointer so we can advance through the packed data
	u8 *pCurrData = (u8 *)*pPackedData;
	
	////////////////////////////////////////////////////////////
	// PACK THE MAIN STRUCT DATA
	////////////////////////////////////////////////////////////
	
	fang_MemCopy( pCurrData, pPortalData, sizeof( FVisData_t ) );
	if ( pPortalData->CellTree.nNodeCount )
		((FVisData_t *)pCurrData)->CellTree.paNodes = (FVisCellTreeNode_t *)((u32)*pPackedData + ADDR_CellTreeNodeData);
	else
		((FVisData_t *)pCurrData)->CellTree.paNodes = NULL;
	if ( pPortalData->nPortalCount )
		((FVisData_t *)pCurrData)->paPortals = (FVisPortal_t *)((u32)*pPackedData + ADDR_PortalData);
	else
		((FVisData_t *)pCurrData)->paPortals = NULL;
	if ( pPortalData->nVolumeCount )
		((FVisData_t *)pCurrData)->paVolumes = (FVisVolume_t *)((u32)*pPackedData + ADDR_VolumeData);
	else
		((FVisData_t *)pCurrData)->paVolumes = NULL;
	if ( pPortalData->nCellCount )
		((FVisData_t *)pCurrData)->paCells = (FVisCell_t *)((u32)*pPackedData + ADDR_CellData);
	else
		((FVisData_t *)pCurrData)->paCells = NULL;
	((FVisData_t *)pCurrData)->nLightCount = nValidLightCount;
	if ( nValidLightCount )
		((FVisData_t *)pCurrData)->paLights = (FLightInit_t *)((u32)*pPackedData + ADDR_LightData);
	else
		((FVisData_t *)pCurrData)->paLights = NULL;
	pCurrData += sizeof( FVisData_t );
	
	////////////////////////////////////////////////////////////
	// PACK THE CELL TREE NODE DATA
	////////////////////////////////////////////////////////////
	
	// Sanity check:
	FASSERT( (u32)pCurrData - (u32)*pPackedData <= ADDR_CellTreeNodeData );
	pCurrData = (u8 *)((u32)*pPackedData + ADDR_CellTreeNodeData);
	
	FVisCellTreeNode_t *pDestTreeNode = (FVisCellTreeNode_t *)pCurrData;
	fang_MemCopy( pCurrData, pPortalData->CellTree.paNodes, sizeof( FVisCellTreeNode_t ) * pPortalData->CellTree.nNodeCount );
	pCurrData += sizeof( FVisCellTreeNode_t ) * pPortalData->CellTree.nNodeCount;
	
	////////////////////////////////////////////////////////////
	// PACK IN THE PORTAL DATA
	////////////////////////////////////////////////////////////
	
	// Sanity check:
	FASSERT( (u32)pCurrData - (u32)*pPackedData <= ADDR_PortalData );
	pCurrData = (u8 *)((u32)*pPackedData + ADDR_PortalData);
	
	if ( pPortalData->nPortalCount )
	{
		FVisPortal_t *pDestPortal = (FVisPortal_t *)pCurrData;
		fang_MemCopy( pDestPortal, pPortalData->paPortals, sizeof( FVisPortal_t ) * pPortalData->nPortalCount );
		pCurrData += sizeof( FVisPortal_t ) * pPortalData->nPortalCount;
	}
	
	////////////////////////////////////////////////////////////
	// PACK IN THE VOLUME DATA
	////////////////////////////////////////////////////////////
	
	// Sanity check:
	FASSERT( (u32)pCurrData - (u32)*pPackedData <= ADDR_VolumeData );
	pCurrData = (u8 *)((u32)*pPackedData + ADDR_VolumeData);
	
	FVisVolume_t *pDestVolume = (FVisVolume_t *)pCurrData;
	fang_MemCopy( pCurrData, pPortalData->paVolumes, sizeof( FVisVolume_t ) * pPortalData->nVolumeCount );
	pCurrData += sizeof( FVisVolume_t ) * pPortalData->nVolumeCount;
	
	// Set some default values and pack the indices
	for ( i = 0; i < pPortalData->nVolumeCount; i++ )
	{
		pDestVolume[i].pUserData = NULL;
		pDestVolume[i].nCrossesPlanesMask = 0;
		for ( ii = 0; ii < FWORLD_TRACKERTYPE_COUNT; ii++ )
		{
			pDestVolume[i].anTotalTrackerCount[ii] = 0;
			pDestVolume[i].aTrackerIntersects[ii].pHeadLink = NULL;
			pDestVolume[i].aTrackerIntersects[ii].pTailLink = NULL;
			pDestVolume[i].aTrackerIntersects[ii].nStructOffset = 0;
			pDestVolume[i].aTrackerIntersects[ii].nCount = 0;
		}
		
		// Pack the volume portal and cell indices
		FVisVolume_t *pSrcVolume = &pPortalData->paVolumes[i];

		if ( pSrcVolume->nPortalCount )
		{
			pDestVolume[i].paPortalIndices = (u16 *)pCurrData;
			fang_MemCopy( pCurrData, pSrcVolume->paPortalIndices, sizeof( u16 ) * pSrcVolume->nPortalCount );
			pCurrData += sizeof( u16 ) * pSrcVolume->nPortalCount;
		}
		else
		{
			pDestVolume[i].paPortalIndices = NULL;
		}
	}
	
	////////////////////////////////////////////////////////////
	// PACK IN THE CELL DATA
	////////////////////////////////////////////////////////////
	
	// Sanity check:
	FASSERT( (u32)pCurrData - (u32)*pPackedData <= ADDR_CellData );
	pCurrData = (u8 *)((u32)*pPackedData + ADDR_CellData);
	
	FVisCell_t *pDestCell = (FVisCell_t *)pCurrData;
	fang_MemCopy( pCurrData, pPortalData->paCells, sizeof( FVisCell_t ) * pPortalData->nCellCount );
	pCurrData += sizeof( FVisCell_t ) * pPortalData->nCellCount;
	
	// Planes need to be specially aligned since they contain CFVec3A's
	// Sanity check:
	FASSERT( (u32)pCurrData - (u32)*pPackedData <= ADDR_CellPlaneData );
	pCurrData = (u8 *)((u32)*pPackedData + ADDR_CellPlaneData);
	
	// Pack the bounding planes
	for ( i = 0; i < pPortalData->nCellCount; i++ )
	{
		pDestCell[i].paBoundingPlanes = (FVisPlane_t *)pCurrData;
		FVisCell_t *pSrcCell = &pPortalData->paCells[i];
		fang_MemCopy( pCurrData, pSrcCell->paBoundingPlanes, sizeof( FVisPlane_t ) * pSrcCell->nPlaneCount );
		pCurrData += sizeof( FVisPlane_t ) * pSrcCell->nPlaneCount;
	}
	
	////////////////////////////////////////////////////////////
	// PACK IN THE LIGHT DATA
	////////////////////////////////////////////////////////////
	
	// Sanity check:
	FASSERT( (u32)pCurrData - (u32)*pPackedData <= ADDR_LightData );
	pCurrData = (u8 *)((u32)*pPackedData + ADDR_LightData);

	fang_MemCopy( pCurrData, pPortalData->paLights, sizeof( FLightInit_t ) * nValidLightCount );
	pCurrData += sizeof( FLightInit_t ) * nValidLightCount;
	
	////////////////////////////////////////////////////////////
	// WE'RE DONE PACKING THE DATA.  NOW WE NEED TO CONVERT THE
	// POINTERS INTO OFFSETS AND CHANGE ENDIAN, IF IT WAS REQUESTED.
	////////////////////////////////////////////////////////////
	
	FVisData_t *pFinishedData = (FVisData_t *)*pPackedData;
	
	// Macro that unfixes up pointers IFF the pointer is non-NULL
	#define __UNFIXUP_POINTER( pointer, type )	if ( pointer ) pointer = (type *)((u32)pointer - (u32)pFinishedData);

	// Handle the Cell Tree
	if ( bChangeEndian )
	{		
		for ( i = 0; i < pFinishedData->CellTree.nNodeCount; i++ )
		{
			pFinishedData->CellTree.paNodes[i].ChangeEndian();
		}
	}
	__UNFIXUP_POINTER( pFinishedData->CellTree.paNodes, FVisCellTreeNode_t );

	// Handle the portals
	if ( bChangeEndian )
	{
		for ( i = 0; i < pFinishedData->nPortalCount; i++ )
		{
			FVisPortal_t *pPortal = &pFinishedData->paPortals[i];
			pPortal->ChangeEndian();
		}
	}
	__UNFIXUP_POINTER( pFinishedData->paPortals, FVisPortal_t );
	
	// Handle the Volumes
	for ( i = 0; i < pFinishedData->nVolumeCount; i++ )
	{
		FVisVolume_t *pVolume = &pFinishedData->paVolumes[i];
		
		if ( bChangeEndian )
		{
			for ( ii = 0; ii < pVolume->nPortalCount; ii++ )
			{
				pVolume->paPortalIndices[ii] = fang_ConvertEndian( pVolume->paPortalIndices[ii] );
			}
		}

		__UNFIXUP_POINTER( pVolume->paPortalIndices, u16 );

		if ( bChangeEndian )
		{
			pVolume->ChangeEndian();
		}
	}
	__UNFIXUP_POINTER( pFinishedData->paVolumes, FVisVolume_t );

	// Handle the cells
	for ( i = 0; i < pFinishedData->nCellCount; i++ )
	{
		FVisCell_t *pCell = &pFinishedData->paCells[i];
		
		if ( bChangeEndian )
		{
			for ( ii = 0; ii < pCell->nPlaneCount; ii++ )
			{
				pCell->paBoundingPlanes[ii].ChangeEndian();
			}
		}
		__UNFIXUP_POINTER( pCell->paBoundingPlanes, FVisPlane_t );
		
		if ( bChangeEndian )
		{
			pCell->ChangeEndian();
		}
	}
	__UNFIXUP_POINTER( pFinishedData->paCells, FVisCell_t );

	// Handle the directional lights
	if ( bChangeEndian )
	{
		for ( i = 0; i < pFinishedData->nLightCount; i++ )
		{
			pFinishedData->paLights[i].ChangeEndian();
		}
	}
	__UNFIXUP_POINTER( pFinishedData->paLights, FLightInit_t );
	
	if ( bChangeEndian )
	{
		pFinishedData->ChangeEndian();
	}
	
	#undef __UNFIXUP_POINTER

	////////////////////////////////////////////////////////////
	// FINISHED!  NOW CROSS YOUR FINGERS AND PRAY THE DATA WORKS!
	////////////////////////////////////////////////////////////
	
	return FVIS_PACK_NO_ERROR;
}


//
//	In-place endian change of raw (non-fixed up) portal data.
//
//		return:
//			TRUE - no error condition encountered
//			FALSE - error condition encountered
//
BOOL fvis_ChangePortalDataEndian( FVisData_t *pRawPortalData )
{
	FASSERT( pRawPortalData );
	
	// Macro that fixes up pointers IFF the pointer is non-NULL
	#define __FIXUP_POINTER( pointer, type )	if ( pointer ) pointer = (type *)((u32)pRawPortalData + (u32)pointer);
	
	// Macro that unfixes up pointers IFF the pointer is non-NULL
	#define __UNFIXUP_POINTER( pointer, type )	if ( pointer ) pointer = (type *)((u32)pointer - (u32)pRawPortalData);

	u32 i, ii;
	
	// Endianize the cell tree
	__FIXUP_POINTER( pRawPortalData->CellTree.paNodes, FVisCellTreeNode_t );
	for ( i = 0; i < pRawPortalData->CellTree.nNodeCount; i++ )
	{
		pRawPortalData->CellTree.paNodes[i].ChangeEndian();
	}
	__UNFIXUP_POINTER( pRawPortalData->CellTree.paNodes, FVisCellTreeNode_t );
	
	// Endianize the portals
	__FIXUP_POINTER( pRawPortalData->paPortals, FVisPortal_t );
	for ( i = 0; i < pRawPortalData->nPortalCount; i++ )
	{
		FVisPortal_t *pPortal = &pRawPortalData->paPortals[i];
		pPortal->ChangeEndian();
	}
	__UNFIXUP_POINTER( pRawPortalData->paPortals, FVisPortal_t );

	// Endianize the volumes
	__FIXUP_POINTER( pRawPortalData->paVolumes, FVisVolume_t );
	for ( i = 0; i < pRawPortalData->nVolumeCount; i++ )
	{
		FVisVolume_t *pVolume = &pRawPortalData->paVolumes[i];
		
		__FIXUP_POINTER( pVolume->paPortalIndices, u16 );
		for ( ii = 0; ii < pVolume->nPortalCount; ii++ )
		{
			pVolume->paPortalIndices[ii] = fang_ConvertEndian( pVolume->paPortalIndices[ii] );
		}
		__UNFIXUP_POINTER( pVolume->paPortalIndices, u16 );

		pVolume->ChangeEndian();
	}
	__UNFIXUP_POINTER( pRawPortalData->paVolumes, FVisVolume_t );

	// Endianize the cells
	__FIXUP_POINTER( pRawPortalData->paCells, FVisCell_t );
	for ( i = 0; i < pRawPortalData->nCellCount; i++ )
	{
		FVisCell_t *pCell = &pRawPortalData->paCells[i];
		
		__FIXUP_POINTER( pCell->paBoundingPlanes, FVisPlane_t );
		for ( ii = 0; ii < pCell->nPlaneCount; ii++ )
		{
			pCell->paBoundingPlanes[ii].ChangeEndian();
		}
		__UNFIXUP_POINTER( pCell->paBoundingPlanes, FVisPlane_t );
		
		pCell->ChangeEndian();
	}
	__UNFIXUP_POINTER( pRawPortalData->paCells, FVisCell_t );

	// Endianize the directional lights
	__FIXUP_POINTER( pRawPortalData->paLights, FLightInit_t );
	for ( i = 0; i < pRawPortalData->nLightCount; i++ )
	{
		pRawPortalData->paLights[i].ChangeEndian();
	}
	__UNFIXUP_POINTER( pRawPortalData->paLights, FLightInit_t );

	pRawPortalData->ChangeEndian();
	
	#undef __FIXUP_POINTER
	#undef __UNFIXUP_POINTER

	return TRUE;	
}


//
//
//
BOOL FVisPortal_t::IntersectRay( const CFVec3 *pStart, const CFVec3 *pEnd, const CFVec3 *pUnitDir )
{
	static u8 __TestAxes[8] = { 1,2,0,2,0,1,0,0 }; // Last two are dummies for alignment
	CFVec3 vStartDiff, vEndDiff;
	vStartDiff = spBoundingWS.m_Pos - *pStart;
	vEndDiff = spBoundingWS.m_Pos - *pEnd;

	// Does it intersect the plane of the portal?
	f32 fStartDot = vStartDiff.Dot( vNormal );
	f32 fEndDot = vEndDiff.Dot( vNormal );

	if ( (fEndDot < 0.f) == (fStartDot < 0.f) )
	{
		// Ry does not cross the plane, so there can be no intersection
		return FALSE;
	}
	
	// Use pythogor. theorum to detemine closest approach of ray
	// Basically, this is an optimized ray v. sphere test
	
	// Calculate hypotenuse ("C squared")
	f32 fCSq = vStartDiff.Mag2();

	// Calculate "B squared" using the ray normal
	f32 fB = vStartDiff.Dot( *pUnitDir );

	// "A squared + B squared = C squared"
	// or here, "A squared = C squared - B squared"
	if ( fCSq - (fB * fB) > fBoundingRadiusSq )
	{
		// If "A squared" is greated than the radius, there can be no intersection
		return FALSE;
	}

	// If there is collision with the sphere, apply the crossings test
	// to determine precisely if the point is in the poly
	u8 nX, nY, i;
	BOOL8 bIntersect = FALSE;
	CFVec3 vPlaneImpact;
	
	i = ( ((nFlags & FVIS_PORTAL_FLAG_NORM_Z_LARGEST) >> 12) - 1) * 2;
	FASSERT( i >= 0 && i < 6 );
	nX = __TestAxes[i];
	nY = __TestAxes[i + 1];

	// Determine the intersection point
	fB = fmath_Div( fStartDot, fStartDot - fEndDot );
	vPlaneImpact.a[nX] = pStart->a[nX] + ((pEnd->a[nX] - pStart->a[nX]) * fB);
	vPlaneImpact.a[nY] = pStart->a[nY] + ((pEnd->a[nY] - pStart->a[nY]) * fB);

	u8 nLastVert = nVertCount - 1;
	BOOL8 bCurrPositive, bLastPositive = (avVertices[nLastVert].a[nY] >= vPlaneImpact.a[nY]);
	for ( i = 0; i < nVertCount; i++ )
	{
		bCurrPositive = (avVertices[i].a[nY] >= vPlaneImpact.a[nY]);

		if ( bLastPositive != bCurrPositive )
		{
			if ( ((avVertices[i].a[nY] - vPlaneImpact.a[nY])*(avVertices[nLastVert].a[nX] - avVertices[i].a[nX]) >= (avVertices[i].a[nX] - vPlaneImpact.a[nX])*(avVertices[nLastVert].a[nY] - avVertices[i].a[nY])) == bCurrPositive )
			{
				// Intersection is on the positive side so we have a crossing
				bIntersect = !bIntersect;
			}
		}

		// Copy over vars for next loop
		bLastPositive = bCurrPositive;
		nLastVert = i;
	}
	
	// If we only crossed one edge in the crossings test, then we
	// know that we have an intersection.  Normally the crossings
	// test would test for an odd number, but since our protals are
	// convex, we know there can only be one crossing.
	return bIntersect;
}

