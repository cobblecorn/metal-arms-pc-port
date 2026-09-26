//////////////////////////////////////////////////////////////////////////////////////
// fworld.cpp - Fang world module.
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
// 12/19/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fworld.h"
#include "fmath.h"
#include "fres.h"
#include "flinklist.h"
#include "fviewport.h"
#include "fvid.h"
#include "fclib.h"
#include "fmesh.h"
#include "fcoll.h"
#include "frenderer.h"
#include "fdraw.h"
#include "fperf.h"
#include "fvis.h"
#include "fworld_coll.h"
#include "fmesh_coll.h"

#include "fresload.h"


#if FANG_PLATFORM_DX
	#include "./dx/fdx8mesh.h"
#elif FANG_PLATFORM_GC
	#include "./gc/fGCmesh.h"
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	#include "./ps2/fps2mesh.h"
//ARG - <<<<<
#else
	#error <Fang Error:  Need to specify mesh h file for platform.>
#endif

#if !FANG_PRODUCTION_BUILD
	#include "ftext.h"
#endif


//////////////////////////////////////////////////////////////////////////////////////
// Local Defines
//////////////////////////////////////////////////////////////////////////////////////

#define _MAX_WORLD_CALLBACK_FCNS		50


//////////////////////////////////////////////////////////////////////////////////////
// Global vars
//////////////////////////////////////////////////////////////////////////////////////

FVisData_t *FWorld_pWorld;

u32 FWorld_nTrackerSkipListCount;
CFWorldTracker *FWorld_apTrackerSkipList[FWORLD_MAX_SKIPLIST_ENTRIES];

CFColorMotif FWorld_AmbientMotif;


//////////////////////////////////////////////////////////////////////////////////////
// Local Global vars
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _bModuleInitialized;
static FWorldInitShapeArrayCallback_t *_pFcnInitShapeArrayCallback;
static BOOL _bToolMode;

static CFColorRGB _AmbientRGB;
static f32 _fAmbientIntensity;

static BOOL _bEnableFog;

static s32 _nWorldCallbackCount;
static FWorldCallback_t **_ppFcnWorldCallbackTable;


//////////////////////////////////////////////////////////////////////////////////////
// Static function prototypes
//////////////////////////////////////////////////////////////////////////////////////

static s32 _FindWorldCallbackFunction( FWorldCallback_t *pFcnWorldCallback );
static void _ResDestroyWorldCallback( void *pBase );


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL fworld_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	FWorld_pWorld = NULL;
	_pFcnInitShapeArrayCallback = NULL;

	_nWorldCallbackCount = 0;
	_ppFcnWorldCallbackTable = (FWorldCallback_t **)fres_Alloc( sizeof(FWorldCallback_t *) * _MAX_WORLD_CALLBACK_FCNS );
	if( _ppFcnWorldCallbackTable == NULL ) 
	{
		return FALSE;
	}
	
	_bToolMode = FALSE;
	_bEnableFog = TRUE;

	CFWorldTracker::InitTrackerLists();

	_bModuleInitialized = TRUE;
	
	// Jump start the visibility system
	if ( !fvis_ModuleStartup() )
	{
		return FALSE;
	}

	// Jump start the world collision system
	if ( !fworld_coll_Startup() )
	{
		return FALSE;
	}

	return TRUE;
}


//
//
//
void fworld_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	_bModuleInitialized = FALSE;

	// Shut down the visibility system
	fvis_ModuleShutdown();

	// Shut down the world collision system
	fworld_coll_Shutdown();
}


//
//
//
void fworld_RegisterWorldCallbackFunction( FWorldCallback_t *pFcnWorldCallback ) 
{
	FASSERT( _bModuleInitialized );

	if( pFcnWorldCallback == NULL ) 
	{
		return;
	}

	FASSERT( _FindWorldCallbackFunction( pFcnWorldCallback ) == -1 );
	FASSERT( _nWorldCallbackCount < _MAX_WORLD_CALLBACK_FCNS );

	_ppFcnWorldCallbackTable[_nWorldCallbackCount++] = pFcnWorldCallback;
}


//
//
//
void fworld_UnregisterWorldCallbackFunction( FWorldCallback_t *pFcnWorldCallback ) 
{
	int i, nIndex;

	FASSERT( _bModuleInitialized );

	if( pFcnWorldCallback == NULL ) 
	{
		return;
	}

	nIndex = _FindWorldCallbackFunction( pFcnWorldCallback );

	if( nIndex != -1 ) 
	{
		// Function found...

		for( i=nIndex+1; i<_nWorldCallbackCount; i++ ) 
		{
			_ppFcnWorldCallbackTable[i-1] = _ppFcnWorldCallbackTable[i];
		}

		_nWorldCallbackCount--;
	}
}


//
//
//
BOOL fworld_IsWorldCallbackFunctionRegistered( FWorldCallback_t *pFcnWorldCallback ) 
{
	FASSERT( _bModuleInitialized );

	if( pFcnWorldCallback == NULL ) 
	{
		return TRUE;
	}

	if( _FindWorldCallbackFunction( pFcnWorldCallback ) != -1 ) 
	{
		return TRUE;
	} 
	else 
	{
		return FALSE;
	}
}


//
//
//
static s32 _FindWorldCallbackFunction( FWorldCallback_t *pFcnWorldCallback ) 
{
	s32 i;

	for( i=0; i<_nWorldCallbackCount; i++ ) 
	{
		if( _ppFcnWorldCallbackTable[i] == pFcnWorldCallback ) 
		{
			// Found it...
			return i;
		}
	}

	// Not found...

	return -1;
}


//
//
//
BOOL fworld_AnnouncePreLoad( void ) 
{
	s32 i;

	for( i=0; i<_nWorldCallbackCount; i++ ) 
	{
		if( !(_ppFcnWorldCallbackTable[i])( FWORLD_EVENT_WORLD_PRELOAD ) ) 
		{
			// Callback returned an error...

			for( i--; i>=0; i-- ) 
			{
				(_ppFcnWorldCallbackTable[i])( FWORLD_EVENT_WORLD_POSTDESTROY );
			}

			return FALSE;
		}
	}

	return TRUE;
}


//
//
//
BOOL fworld_AnnouncePostLoad( void ) 
{
	s32 i;

	for( i=0; i<_nWorldCallbackCount; i++ ) 
	{
		if( !(_ppFcnWorldCallbackTable[i])( FWORLD_EVENT_WORLD_POSTLOAD ) ) 
		{
			// Callback returned an error...

			for( i--; i>=0; i-- ) 
			{
				(_ppFcnWorldCallbackTable[i])( FWORLD_EVENT_WORLD_PREDESTROY );
			}

			return FALSE;
		}
	}

	return TRUE;
}


//
//
//
void fworld_AnnouncePreDestroy( void ) 
{
	s32 i;

	for( i=_nWorldCallbackCount-1; i>=0; i-- ) 
	{
		(_ppFcnWorldCallbackTable[i])( FWORLD_EVENT_WORLD_PREDESTROY );
	}
}


//
//
//
void fworld_AnnouncePostDestroy( void ) 
{
	s32 i;

	for( i=_nWorldCallbackCount-1; i>=0; i-- ) 
	{
		(_ppFcnWorldCallbackTable[i])( FWORLD_EVENT_WORLD_POSTDESTROY );
	}
}


//
//
//
FWorldInitShapeArrayCallback_t *fworld_SetShapeCreateCallback( FWorldInitShapeArrayCallback_t *pInitShapeCallback ) 
{
	FWorldInitShapeArrayCallback_t *pPrevValue;

	FASSERT( _bModuleInitialized );

	pPrevValue = _pFcnInitShapeArrayCallback;
	_pFcnInitShapeArrayCallback = pInitShapeCallback;

	return pPrevValue;
}


//
//
//
FWorldInitShapeArrayCallback_t *fworld_GetMeshCreateCallback( void ) 
{
	FASSERT( _bModuleInitialized );
	return _pFcnInitShapeArrayCallback;
}


//
//
//
static void _ResDestroyWorldCallback( void *pBase ) 
{
	pBase;
	fworld_AnnouncePreDestroy();
	CFWorldTracker::DeleteAllFlagedTrackers();
	FWorld_pWorld = NULL;
	fworld_AnnouncePostDestroy();
}


//
//
//
BOOL fworld_CreateGameObjects( cchar *pszWorldResName, const void *pFixupOffsetBase, const CFWorldShapeInit *pShapeInitArray, u32 nShapeInitCount ) 
{
	const CFWorldShapeInit *pShapeInit;
	const CFWorldShapeMesh *pShapeMesh;
	CFWorldMesh *pWorldMesh;
	FMeshInit_t MeshInit;
	cchar *pszMeshName;
	u32 i, j;

	FASSERT( _bModuleInitialized );
	FASSERT( FWorld_pWorld );

	// Fixup the pointers...
	for( i=0, pShapeInit=pShapeInitArray; i<nShapeInitCount; i++, pShapeInit++ ) 
	{
		if( pShapeInit->m_pShape ) 
		{
			*(u32 *)&pShapeInit->m_pShape = ( (u32)pFixupOffsetBase + (u32)pShapeInit->m_pShape );
		}

		if( pShapeInit->m_pGameData ) 
		{
			*(u32 *)&pShapeInit->m_pGameData = ( (u32)pFixupOffsetBase + (u32)pShapeInit->m_pGameData );
		}

		switch( pShapeInit->m_nShapeType ) 
		{
			case FWORLD_SHAPETYPE_SPLINE:
				{
					CFWorldShapeSpline *pSpline = (CFWorldShapeSpline *)pShapeInit->m_pSpline;
					if( pSpline->m_pPtArray ) 
					{
						*(u32 *)&pSpline->m_pPtArray = ( (u32)pFixupOffsetBase + (u32)pSpline->m_pPtArray );
					}
				}

				break;

			case FWORLD_SHAPETYPE_MESH:
				{
					CFWorldShapeMesh *pMesh = (CFWorldShapeMesh *)pShapeInit->m_pMesh;
					if( pMesh->m_pszMeshName ) 
					{
						*(u32 *)&pMesh->m_pszMeshName = ( (u32)pFixupOffsetBase + (u32)pMesh->m_pszMeshName );
					}

					for ( j = 0; j < FWORLD_MAX_MESH_SHAPE_LIGHTMAPS; j++ )
					{
						if( pMesh->m_papszLightMapName[j] ) 
						{
							*(u32 *)&pMesh->m_papszLightMapName[j] = ( (u32)pFixupOffsetBase + (u32)pMesh->m_papszLightMapName[j] );
						}
					}

					pMesh->m_paColorStreams = (ColorStream_t *)( (u32)pFixupOffsetBase + (u32)pMesh->m_paColorStreams );
					for ( j = 0; j < pMesh->m_nColorStreamCount; j++ )
					{
						if( pMesh->m_paColorStreams[j].paVertexColors ) 
						{
							pMesh->m_paColorStreams[j].paVertexColors = (void *)( (u32)pFixupOffsetBase + (u32)pMesh->m_paColorStreams[j].paVertexColors );
						}
					}
				}

				break;

		}
	}

	if( _pFcnInitShapeArrayCallback ) 
	{
		// A handler callback exists...
		return _pFcnInitShapeArrayCallback( pszWorldResName, pFixupOffsetBase, pShapeInitArray, nShapeInitCount );
	}

	// A handler callback doesn't exist.
	// Let's create meshes ourselves...

	for( i=0, pShapeInit=pShapeInitArray; i<nShapeInitCount; i++, pShapeInit++ ) 
	{
		if( pShapeInit->m_nShapeType==FWORLD_SHAPETYPE_MESH ) 
		{
			pShapeMesh = pShapeInit->m_pMesh;

			if( pShapeMesh ) 
			{
				MeshInit.Mtx.Set( pShapeInit->m_Mtx43 );
				MeshInit.fCullDist = fmath_Sqrt( pShapeMesh->m_fCullDist2 );
				MeshInit.nFlags = pShapeMesh->m_nMeshInstFlags;

				pszMeshName = pShapeMesh->m_pszMeshName;

				MeshInit.pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshName );
				if( MeshInit.pMesh == NULL ) 
				{
					// Mesh resource not found...
					DEVPRINTF( "fworld_CreateGameObjects(): Could not load mesh '%s' for world '%s'.\n", pszMeshName, pszWorldResName );
					continue;
				}

				pWorldMesh = fnew CFWorldMesh;
				if( pWorldMesh == NULL ) 
				{
					DEVPRINTF( "fworld_CreateGameObjects(): Could not allocate memory for all CFWorldMesh objects.\n" );
					break;
				}

				pWorldMesh->Init( &MeshInit );
				pWorldMesh->SetCollisionFlag( !(pWorldMesh->m_nFlags & FMESHINST_FLAG_NOCOLLIDE) );
			}
		}
	}

	return TRUE;
}


//
//
//
BOOL fworld_GetToolMode( void ) 
{
	FASSERT( _bModuleInitialized );
	return _bToolMode;
}

// Important: When tool mode is enabled, fworld assumes that FWorldNode_t::pnDisplayListBitStream points
//            to an array of u16 indices into FWorld_t::pNodeArray. This array represents the display list
//            for the leaf, and is terminated by a 0xffff value.
BOOL fworld_SetToolMode( BOOL bEnableToolMode ) 
{
	BOOL bPrevious;

	FASSERT( _bModuleInitialized );

	bPrevious = _bToolMode;
	_bToolMode = bEnableToolMode;

	return bPrevious;
}


//
//
//
void fworld_Ambient_Set( float fRed, float fGreen, float fBlue, float fIntensity ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT_UNIT_FLOAT( fRed );
	FASSERT_UNIT_FLOAT( fGreen );
	FASSERT_UNIT_FLOAT( fBlue );
	FASSERT_UNIT_FLOAT( fIntensity );

	_AmbientRGB.Set( fRed, fGreen, fBlue );
	_fAmbientIntensity = fIntensity;

	FWorld_AmbientMotif.ColorRGB = _AmbientRGB * _fAmbientIntensity;
}


//
//
//
void fworld_Ambient_Set( const CFColorRGB *pColorRGB, float fIntensity ) 
{
	FASSERT( _bModuleInitialized );
	FCOLOR_FASSERT_UNIT_RGB( pColorRGB );
	FASSERT_UNIT_FLOAT( fIntensity );

	_AmbientRGB = *pColorRGB;
	_fAmbientIntensity = fIntensity;

	FWorld_AmbientMotif.ColorRGB = _AmbientRGB * _fAmbientIntensity;
}


//
//
//
void fworld_Ambient_Get( CFColorRGB *pColorRGB, float *pfIntensity ) 
{
	FASSERT( _bModuleInitialized );

	if ( pColorRGB ) 
	{
		*pColorRGB = _AmbientRGB;
	}
	if ( pfIntensity ) 
	{
		*pfIntensity = _fAmbientIntensity;
	}
}


//
//
//
BOOL fworld_EnableFog( BOOL bEnable ) 
{
	BOOL bPrevState;

	FASSERT( _bModuleInitialized );

	bPrevState = _bEnableFog;
	_bEnableFog = bEnable;
	return bPrevState;
}


//
//
//
BOOL fworld_IsFogEnabled( void ) 
{
	FASSERT( _bModuleInitialized );
	return _bEnableFog;
}


//
// Computes the dynamic light color at the specified point.
//
// Returns TRUE if the point is within the world bounds.
// Returns FALSE if the point is outside the world bounds.
//
// If the point is outside the world bounds, pDestColor is filled with black.
BOOL fworld_LightPoint( const CFVec3A *pPoint_WS, CFColorRGB *pDestColor, BOOL bIncludeAmbient, const CFWorldTracker *pStartHint/*=NULL*/ ) 
{
	FVisVolume_t *pVolume;

	FASSERT( _bModuleInitialized );
	FASSERT( FWorld_pWorld );

	// Start out with the ambient, if requested
	if( bIncludeAmbient ) 
	{
		*pDestColor = FWorld_AmbientMotif.ColorRGB;
	} 
	else 
	{
		pDestColor->Black();
	}

	// Find the relevant volume
	fworld_GetVolumeContainingPoint( pPoint_WS, &pVolume, pStartHint );
	if( pVolume == NULL ) 
	{
		// Point is outside the world...
		return FALSE;
	}

	// Find all world lights that intersect this volume...
	CFLight *pLight;
	CFWorldLight *pWorldLight;
	CFColorRGB LitPointColorRGB;
	CFWorldIntersect *pTestInt = NULL;
	FLinkRoot_t *pRootInt = &pVolume->aTrackerIntersects[FWORLD_TRACKERTYPE_LIGHT];
	while( (pTestInt = (CFWorldIntersect *)flinklist_GetNext( pRootInt, pTestInt )) ) 
	{
		pWorldLight = (CFWorldLight *)pTestInt->GetTracker();
		pLight = &pWorldLight->m_Light;

		if( !(pLight->m_nFlags & FLIGHT_FLAG_ENABLE) ) 
		{
			// Light is disabled...
			continue;
		}

		if( !pWorldLight->GetBoundingSphere().IsIntersecting( pPoint_WS->v3 ) ) 
		{
			// Light doesn't intersect point...
			continue;
		}

		// Light intersects this point...
		if( pLight->m_nType == FLIGHT_TYPE_OMNI || pLight->m_nType == FLIGHT_TYPE_SPOT ) 
		{
			pLight->LightPoint( pPoint_WS, &LitPointColorRGB );
			*pDestColor += LitPointColorRGB;
		}
	}

	// Clamp color...
	pDestColor->Clamp1();

	return TRUE;
}


//
// Computes the dynamic light color at the specified point.
//
// Returns TRUE if the point is within the world bounds.
// Returns FALSE if the point is outside the world bounds.
//
// If the point is outside the world bounds, pDestColor is filled with black.
BOOL fworld_LightDirectionalPoint( const CFVec3A *pPoint_WS, const CFVec3A *pUnitNormal_WS, CFColorRGB *pDestColor, BOOL bIncludeAmbient, const CFWorldTracker *pStartHint/*=NULL*/ ) 
{
	FVisVolume_t *pVolume;

	FASSERT( _bModuleInitialized );
	FASSERT( FWorld_pWorld );

	if( bIncludeAmbient ) 
	{
		*pDestColor = FWorld_AmbientMotif.ColorRGB;
	} 
	else 
	{
		pDestColor->Black();
	}

	fworld_GetVolumeContainingPoint( pPoint_WS, &pVolume, pStartHint );

	if( pVolume == NULL ) 
	{
		// Point is outside the world...
		return FALSE;
	}

	// Find all world lights that intersect this volume...
	CFLight *pLight;
	CFWorldLight *pWorldLight;
	CFColorRGB LitPointColorRGB;
	CFWorldIntersect *pTestInt = NULL;
	FLinkRoot_t *pRootInt = &pVolume->aTrackerIntersects[FWORLD_TRACKERTYPE_LIGHT];
	while( (pTestInt = (CFWorldIntersect *)flinklist_GetNext( pRootInt, pTestInt )) ) 
	{
		pWorldLight = (CFWorldLight *)pTestInt->GetTracker();
		pLight = &pWorldLight->m_Light;

		if( !(pLight->m_nFlags & FLIGHT_FLAG_ENABLE) ) 
		{
			// Light is disabled...
			continue;
		}

		if( !pWorldLight->GetBoundingSphere().IsIntersecting( pPoint_WS->v3 ) ) 
		{
			// Light doesn't intersect point...
			continue;
		}

		// Light intersects this point...
		if( pLight->m_nType==FLIGHT_TYPE_OMNI || pLight->m_nType==FLIGHT_TYPE_SPOT ) 
		{
			pLight->LightDirectionalPoint( pPoint_WS, pUnitNormal_WS, &LitPointColorRGB );
			*pDestColor += LitPointColorRGB;
		}
	}

	// Clamp color...
	pDestColor->Clamp1();

	return TRUE;
}


//-------------------------------------------------------------------------------------------------------------------
// CFWorldKey:
//-------------------------------------------------------------------------------------------------------------------
BOOL CFWorldKey::m_abInUse[FWORLD_KEYCOUNT];
u32 CFWorldKey::m_anGlobalVisitedKey[FWORLD_KEYCOUNT];



//-------------------------------------------------------------------------------------------------------------------
// CFWorldTracker:
//-------------------------------------------------------------------------------------------------------------------

//
//
//
CFWorldTracker::CFWorldTracker( FWorldTrackerType_e nTrackerType ) 
{
	m_nTrackerType = nTrackerType;
	m_nTrackerFlags = 0;
	flinklist_InitRoot( &m_IntersectList, (s32)FANG_OFFSETOF( CFWorldIntersect, m_NodeInTrackerLink ) );
	m_BoundSphere_WS.Zero();
	m_pUser = NULL;
	m_nUserTypeBits = 0x8000000000000000;
	m_nUser = 0;
	m_nLastFrameMoved = FVid_nFrameCounter - 1;

	if( FWorld_pWorld ) 
	{
		// The world has been created...
		flinklist_AddTail( &m_TrackerDelRoot, this );
		SetAutoDelFlag( TRUE );
	}
}

//
//
//
CFWorldTracker::~CFWorldTracker( void ) 
{
	RemoveFromWorld();

	if( IsAutoDelFlagSet() ) 
	{
		flinklist_Remove( &m_TrackerDelRoot, this );
		SetAutoDelFlag( FALSE );
	}
}


//
//
//
void CFWorldTracker::DeleteAllFlagedTrackers( void ) 
{
	CFWorldTracker *pWorldTracker;

	while( pWorldTracker = GetLastTracker() ) 
	{
		FASSERT( pWorldTracker->IsAddedToWorld() );
		pWorldTracker->RemoveFromWorld();
	}

	while( pWorldTracker = GetLastTrackerDel() ) 
	{
		FASSERT( pWorldTracker->IsAutoDelFlagSet() );
		fdelete( pWorldTracker );
	}
}

//
//
//
#define _MAX_ADD_TO_WORLD_NODES		256
static FVisVolume_t *_papAddVolumeList[_MAX_ADD_TO_WORLD_NODES];
static u16 _nAddCount;
static BOOL _AddToWorld_VolCallback( FVisVolume_t *pVolume, BOOL bUsingVisData )
{
	if ( _nAddCount == _MAX_ADD_TO_WORLD_NODES )
	{
		FASSERT_NOW;
		return FALSE;
	}

#if FANG_DEBUG_BUILD
	u32 i;
	// Verify that we don't already have this volume in the list
	for ( i = 0; i < _nAddCount; i++ )
	{
		FASSERT( _papAddVolumeList[i] != pVolume );
	}
#endif
	_papAddVolumeList[_nAddCount++] = pVolume;

	return TRUE;
}

//
//
//
void CFWorldTracker::AddToWorld( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( m_BoundSphere_WS.m_fRadius >= 0.0f );
	
	u32 i;
	_nAddCount = 0;
	_papAddVolumeList[0] = NULL;

//	if ( GetType()== FWORLD_TRACKERTYPE_MESH && fclib_strnicmp(((CFWorldMesh *)this)->m_pMesh->szName, "grdg", 4 ) == 0 )
//	{
//		_nAddCount = 0;
//	}
	
	if ( GetIntersectCount() == 1 && GetType() != FWORLD_TRACKERTYPE_LIGHT )
	{
		// In this instance, we can attempt to perform some optimizations

		// Pass a hint to the vis system as to where the tracker may reside
		fworld_GetVolumesIntersectingSphere( &m_BoundSphere_WS, _AddToWorld_VolCallback, this );
/*
		if ( GetType()== FWORLD_TRACKERTYPE_MESH && fclib_strnicmp(((CFWorldMesh *)this)->m_pMesh->szName, "grdm", 4 ) == 0 )
		{
			if ( _papAddNodeList[0]->nVolumeID != 3 )
			{
				_papAddNodeList[3]->nVolumeID = 12;
			}
			ftext_DebugPrintf( 0.5, 0.5, "Vol: %d - Col: %d", _papAddNodeList[0]->nVolumeID, _nAddCount );
		}
*/		
		// If the tracker is in one volume and hasn't changed, no need to update
		if ( _nAddCount == 1 && GetFirstIntersect()->GetVolume() == _papAddVolumeList[0] )
		{
			return;
		}

		// We couldn't bail quickly, so reset the intersect list
		RemoveFromWorld();

		for ( i = 0; i < _nAddCount; i++ )
		{
			CFWorldIntersect::Create( this, _papAddVolumeList[i] );
//			_papAddVolumeList[i]->AddTrackerDeltaToCount( GetType(), 1 );
		}

	}
	else
	{
		// We've got other than one intersect, so just remove (if necessary) and re-add
		if ( IsAddedToWorld() )
		{
			RemoveFromWorld();
		}
#if 0
		if ( GetType() == FWORLD_TRACKERTYPE_LIGHT )
		{
			fworld_GetVolumesIntersectingSphere( &m_BoundSphere_WS, _AddToWorld_VolCallback, this, -1, FALSE );
		}
		else
#endif
		{
			fworld_GetVolumesIntersectingSphere( &m_BoundSphere_WS, _AddToWorld_VolCallback, this );
		}

		for ( i = 0; i < _nAddCount; i++ )
		{
			CFWorldIntersect::Create( this, _papAddVolumeList[i] );
//			_papAddVolumeList[i]->AddTrackerDeltaToCount( GetType(), 1 );
		}
	}

	FMATH_SETBITMASK( m_nTrackerFlags, FWORLD_TRACKER_FLAG_ADDED );
	flinklist_AddTail( &m_TrackerRoot, this );
}


//
//
//
void CFWorldTracker::RemoveFromWorld( void ) 
{
	CFWorldIntersect *pIntersect;

	if ( IsAddedToWorld() ) { 
		FASSERT( FWorld_pWorld != NULL );
		FASSERT( _bModuleInitialized );

		while( (pIntersect = GetFirstIntersect()) ) 
		{
			FASSERT( pIntersect->m_pTracker == this );
//			pIntersect->GetVolume()->AddTrackerDeltaToCount( GetType(), -1 );
			pIntersect->Destroy();
		}

		flinklist_Remove( &m_TrackerRoot, this );

		FMATH_CLEARBITMASK( m_nTrackerFlags, FWORLD_TRACKER_FLAG_ADDED );
	}
}


//
// Return FALSE if the callback signalled to bail early.
//
BOOL CFWorldTracker::FindIntersectingTrackers( FWorldIntersectingTrackerCallback_t *pCallback ) 
{
	u32 i;
	CFWorldIntersect *pSrcIntersect, *pDstIntersect;
	CFWorldTracker *pIntersectingTracker;
	FVisVolume_t *pVolume;
	FLinkRoot_t *pRootIntersect;
	s32 nKeyIndex;

	FASSERT( _bModuleInitialized );
	FASSERT( FWorld_pWorld != NULL );

	nKeyIndex = CFWorldKey::OpenKey();
	if ( nKeyIndex == -1 ) 
	{
		DEVPRINTF( "CFWorldTracker::FindIntersectingTrackers(): No more world keys available.\n" );
		return TRUE;
	}

	pSrcIntersect = NULL;
	while ( (pSrcIntersect = GetNextIntersect( pSrcIntersect )) ) 
	{
		// Each intersect represents one leaf node that this object most likely intersects.

		pVolume = pSrcIntersect->GetVolume();

		for ( i = 0; i < FWORLD_TRACKERTYPE_COUNT; i++ )
		{
			pRootIntersect = &pVolume->aTrackerIntersects[i];

			u32 nVolTrackerKey = FVis_anVolumeTrackerKey[i];

			pDstIntersect = NULL;
			while ( (pDstIntersect = (CFWorldIntersect *)flinklist_GetNext( pRootIntersect, pDstIntersect )) ) 
			{
				pIntersectingTracker = pDstIntersect->GetTracker();

				if (!pIntersectingTracker)
				{
					continue;
				}

				// Check to see if we've already visited this tracker...
				if ( pIntersectingTracker->m_VisitedKey.HaveVisited( nKeyIndex ) ) 
				{
					// We've already visited the intersecting tracker, so let's skip it...
					continue;
				}

				// Flag as visited...
				pIntersectingTracker->m_VisitedKey.FlagAsVisited( nKeyIndex );

				// Make sure this tracker isn't ourselves...
				if ( pIntersectingTracker == this ) 
				{
					// The intersecting tracker is really ourselves, so let's skip it...
					continue;
				}

				// Check to see if the two trackers' spheres intersect...
				if ( !m_BoundSphere_WS.IsIntersecting( pIntersectingTracker->m_BoundSphere_WS ) ) 
				{
					// Trackers do not intersect...
					continue;
				}

				// We've found a potentially-intersecting tracker!

				if ( !pCallback( pIntersectingTracker, pVolume ) ) 
				{
					// Caller returned FALSE, which means we should not continue this function...
					CFWorldKey::CloseKey( nKeyIndex );
					return FALSE;
				}
				else if ( nVolTrackerKey != FVis_anVolumeTrackerKey[i] )
				{
					// The callback functions changed the tracker list in the volume we are traversing.  We must bail out for safety
					FASSERT_NOW;
					CFWorldKey::CloseKey( nKeyIndex );
					return TRUE;
				}
			}
		}
	}

	CFWorldKey::CloseKey( nKeyIndex );

	return TRUE;
}

//
// Return FALSE if the callback signalled to bail early.
//
BOOL CFWorldTracker::FindIntersectingTrackers( FWorldIntersectingTrackerCallback_t *pCallback, FWorldTrackerType_e nTypeToFind ) 
{
	CFWorldIntersect *pSrcIntersect, *pDstIntersect;
	CFWorldTracker *pIntersectingTracker;
	FVisVolume_t *pVolume;
	FLinkRoot_t *pRootIntersect;
	s32 nKeyIndex;

	FASSERT( _bModuleInitialized );
	FASSERT( FWorld_pWorld != NULL );
	
	nKeyIndex = CFWorldKey::OpenKey();
	if( nKeyIndex == -1 ) 
	{
		DEVPRINTF( "CFWorldTracker::FindIntersectingTrackers(): No more world keys available.\n" );
		return TRUE;
	}

	u32 nVolTrackerKey = FVis_anVolumeTrackerKey[nTypeToFind];

	pSrcIntersect = NULL;
	while( (pSrcIntersect = GetNextIntersect( pSrcIntersect )) ) 
	{
		// Each intersect represents one leaf node that this object most likely intersects.

		pVolume = pSrcIntersect->GetVolume();
		pRootIntersect = &pVolume->aTrackerIntersects[nTypeToFind];

		pDstIntersect = NULL;
		while ( (pDstIntersect = (CFWorldIntersect *)flinklist_GetNext( pRootIntersect, pDstIntersect )) ) 
		{
			pIntersectingTracker = pDstIntersect->GetTracker();

			// Check to see if we've already visited this tracker...
			if ( pIntersectingTracker->m_VisitedKey.HaveVisited( nKeyIndex ) ) 
			{
				// We've already visited the intersecting tracker, so let's skip it...
				continue;
			}

			// Flag as visited...
			pIntersectingTracker->m_VisitedKey.FlagAsVisited( nKeyIndex );

			// Make sure this tracker isn't ourselves...
			if ( pIntersectingTracker == this ) 
			{
				// The intersecting tracker is really ourselves, so let's skip it...
				continue;
			}

			// Check to see if the two trackers' spheres intersect...
			if ( !m_BoundSphere_WS.IsIntersecting( pIntersectingTracker->m_BoundSphere_WS ) ) 
			{
				// Trackers do not intersect...
				continue;
			}

			// We've found a potentially-intersecting tracker!

			if ( !pCallback( pIntersectingTracker, pVolume ) ) 
			{
				// Caller returned FALSE, which means we should not continue this function...
				CFWorldKey::CloseKey( nKeyIndex );
				return FALSE;
			}
			else if ( nVolTrackerKey != FVis_anVolumeTrackerKey[nTypeToFind] )
			{
				// The callback changed the trackers in the volume we are traversing.  We must bail out for safety
				FASSERT_NOW;
				CFWorldKey::CloseKey( nKeyIndex );
				return TRUE;
			}
		}
	}
/*
	pSrcIntersect = NULL;
	while( (pSrcIntersect = GetNextIntersect( pSrcIntersect )) ) {
		// Each intersect represents one leaf node that this object most likely intersects.

		pWorldLeafNode = pSrcIntersect->GetWorldLeafNode();

		pLeafRoot = &pWorldLeafNode->TrackerIntersects;

#if 1
		pDstIntersect = (CFWorldIntersect *)flinklist_GetNext( pLeafRoot, NULL );
		while( pDstIntersect ) 
		{
			pIntersectingTracker = pDstIntersect->GetTracker();
			pNextIntersect = (CFWorldIntersect *)flinklist_GetNext( pLeafRoot, pDstIntersect );

			// Check to see if this tracker is of the desired type...
			if( pIntersectingTracker->GetType() == nTypeToFind ) 
			{
				// Check to see if we've already visited this tracker...
				if( !pIntersectingTracker->m_VisitedKey.HaveVisited(nKeyIndex) ) 
				{
					// Flag as visited...
					pIntersectingTracker->m_VisitedKey.FlagAsVisited(nKeyIndex);

					// Make sure this tracker isn't ourselves...
					if( pIntersectingTracker != this ) 
					{
						// Check to see if the two trackers' spheres intersect...
						if( m_TightBoundSphere_WS.IsIntersecting( pIntersectingTracker->m_TightBoundSphere_WS ) ) 
						{
							// We've found a potentially-intersecting tracker!

							if( !pCallback( pIntersectingTracker, pWorldLeafNode ) ) 
							{
								// Caller returned FALSE, which means we should not continue this function...
								CFWorldKey::CloseKey( nKeyIndex );
								return FALSE;
							}
						}
					}
				}

			}

			pDstIntersect = pNextIntersect;
		}

#else
		pDstIntersect = NULL;
		while( (pDstIntersect = (CFWorldIntersect *)flinklist_GetNext( pLeafRoot, pDstIntersect )) ) {
			pIntersectingTracker = pDstIntersect->GetTracker();

			// Check to see if this tracker is of the desired type...
			if( pIntersectingTracker->GetType() != nTypeToFind ) {
				// The intersecting tracker is of the wrong type, so let's skip it...
				continue;
			}

			// Check to see if we've already visited this tracker...
			if( pIntersectingTracker->m_VisitedKey.HaveVisited(nKeyIndex) ) {
				// We've already visited the intersecting tracker, so let's skip it...
				continue;
			}

			// Flag as visited...
			pIntersectingTracker->m_VisitedKey.FlagAsVisited(nKeyIndex);

			// Make sure this tracker isn't ourselves...
			if( pIntersectingTracker == this ) {
				// The intersecting tracker is really ourselves, so let's skip it...
				continue;
			}

			// Check to see if the two trackers' spheres intersect...
			if( !m_TightBoundSphere_WS.IsIntersecting( pIntersectingTracker->m_TightBoundSphere_WS ) ) {
				// Trackers do not intersect...
				continue;
			}

			// We've found a potentially-intersecting tracker!

			if( !pCallback( pIntersectingTracker, pWorldLeafNode ) ) {
				// Caller returned FALSE, which means we should not continue this function...
				CFWorldKey::CloseKey( nKeyIndex );
				return FALSE;
			}
		}
#endif
	}
*/
	CFWorldKey::CloseKey( nKeyIndex );

	return TRUE;
}

//
//
//
u32 CFWorldTracker::GetVolumeFlags( void ) 
{
	u32 nFlags = 0;

	CFWorldIntersect *pSrcIntersect = NULL;
	while( (pSrcIntersect = GetNextIntersect( pSrcIntersect )) ) 
	{
		nFlags |= FVis_anVolumeFlags[ pSrcIntersect->GetVolume()->nVolumeID ];
	}

	return nFlags;
}

//
//
//
BOOL CFWorldTracker::VolumeBecameActive( void )
{
	u16 nVolumeID;
	u32 nFlags = 0;
	u32 nPreviousFlags = 0;

	CFWorldIntersect *pSrcIntersect = NULL;
	while( (pSrcIntersect = GetNextIntersect( pSrcIntersect )) ) 
	{
		nVolumeID = pSrcIntersect->GetVolume()->nVolumeID;
		nFlags |= FVis_anVolumeFlags[ nVolumeID ];
		nPreviousFlags |= FVis_anPreviousVolumeFlags[ nVolumeID ];
	}

	if( (nFlags & FVIS_VOLUME_IN_ACTIVE_LIST) && !(nPreviousFlags & FVIS_VOLUME_IN_ACTIVE_LIST) )
	{
		return TRUE;
	}

	return FALSE;
}

//
//
//
BOOL CFWorldTracker::VolumeBecameInactive( void )
{
	u16 nVolumeID;
	u32 nFlags = 0;
	u32 nPreviousFlags = 0;

	CFWorldIntersect *pSrcIntersect = NULL;
	while( (pSrcIntersect = GetNextIntersect( pSrcIntersect )) ) 
	{
		nVolumeID = pSrcIntersect->GetVolume()->nVolumeID;
		nFlags |= FVis_anVolumeFlags[ nVolumeID ];
		nPreviousFlags |= FVis_anPreviousVolumeFlags[ nVolumeID ];
	}

	if( !(nFlags & FVIS_VOLUME_IN_ACTIVE_LIST) && (nPreviousFlags & FVIS_VOLUME_IN_ACTIVE_LIST) )
	{
		return TRUE;
	}

	return FALSE;
}

//
//
//
void CFWorldTracker::InitTrackerLists( void ) 
{
	flinklist_InitRoot( &m_TrackerRoot, CFWorldTracker::GetOffsetOfTrackerLink() );
	flinklist_InitRoot( &m_TrackerDelRoot, CFWorldTracker::GetOffsetOfTrackerDelLink() );
}


FLinkRoot_t CFWorldTracker::m_TrackerRoot;
FLinkRoot_t CFWorldTracker::m_TrackerDelRoot;


//-------------------------------------------------------------------------------------------------------------------
// CFWorldIntersect:
//-------------------------------------------------------------------------------------------------------------------
CFWorldIntersect *CFWorldIntersect::m_pIntersectFreePool;
FLinkRoot_t CFWorldIntersect::m_FreeIntersectRoot;


//
//
//
void CFWorldIntersect::AddToWorld( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( pTracker && pVolume );

	m_pVolume = pVolume;
	m_pTracker = pTracker;

	flinklist_AddTail( &pTracker->m_IntersectList, this );
	m_pVolume->AddIntersect( this, m_pTracker->GetType() );
}


//
//
//
void CFWorldIntersect::RemoveFromWorld( void ) 
{
	FASSERT( _bModuleInitialized );

	if( m_pTracker ) 
	{
		flinklist_Remove( &m_pTracker->m_IntersectList, this );

		m_pVolume->RemoveIntersect( this, m_pTracker->GetType() );
		m_pTracker = NULL;
	}
}


//
//
//
void CFWorldIntersect::InitPool( CFWorldIntersect *pPoolBase, u32 nMaxIntersects ) 
{
	m_pIntersectFreePool = pPoolBase;

	flinklist_InitRoot( &m_FreeIntersectRoot, FANG_OFFSETOF( CFWorldIntersect, m_PoolLink ) );
	flinklist_InitPool( &m_FreeIntersectRoot, m_pIntersectFreePool, sizeof(CFWorldIntersect), (s32)nMaxIntersects );
}



//-------------------------------------------------------------------------------------------------------------------
// CFWorldLight:
//-------------------------------------------------------------------------------------------------------------------
CFWorldLight *CFWorldLight::FindByName( cchar *pszLightName ) 
{
	CFWorldTracker *pTracker;
	CFWorldLight *pWorldLight;

	for( pTracker=CFWorldTracker::GetFirstTracker(); pTracker; pTracker=CFWorldTracker::GetNextTracker(pTracker) ) 
	{
		if( pTracker->GetType() == FWORLD_TRACKERTYPE_LIGHT ) 
		{
			pWorldLight = (CFWorldLight *)pTracker;

			if( !fclib_stricmp( pszLightName, pWorldLight->m_Light.m_szName ) ) 
			{
				return pWorldLight;
			}
		}
	}

	return NULL;
}

void CFWorldLight::GetInit( FLightInit_t *pLightInit ) 
{
	if (!pLightInit)
	{
		DEVPRINTF("Invalid pointer passed to CFWorldLight::GetInit()\n");
		return;
	}
	pLightInit->fCoronaScale =		m_Light.m_fCoronaScale;
	pLightInit->fIntensity	 =		m_Light.GetIntensity();
	pLightInit->fSpotInnerRadians = m_Light.m_fSpotInnerRadians;
	pLightInit->fSpotOuterRadians = m_Light.m_fSpotOuterRadians;
	pLightInit->Motif			  = m_Light.m_Motif;
	
	pLightInit->mtxOrientation	= m_Light.m_mtxOrientation_WS;
	pLightInit->spInfluence		= m_Light.m_spInfluence_WS;

	pLightInit->nFlags			  = m_Light.m_nFlags;
	pLightInit->nLightID		  = 0xffff;
	pLightInit->nParentBoneIdx	  = -1;
	pLightInit->nType			  = m_Light.m_nType;
	
	pLightInit->szCoronaTexName[0]=0;
	if (m_Light.m_pCoronaTex && m_Light.m_pCoronaTex->GetTexDef()) 
	{
		fclib_strcpy(pLightInit->szCoronaTexName, m_Light.m_pCoronaTex->GetTexDef()->TexInfo.szName);
	}

	pLightInit->szPerPixelTexName[0]=0;
	if (m_Light.m_pProjectionTex && m_Light.m_pProjectionTex->GetTexDef())
	{
		fclib_strcpy(pLightInit->szPerPixelTexName, m_Light.m_pProjectionTex->GetTexDef()->TexInfo.szName);
	}
}


//-------------------------------------------------------------------------------------------------------------------
// CFWorldMeshDirLight:
//-------------------------------------------------------------------------------------------------------------------

//
//
//
CFWorldMeshDirLight::CFWorldMeshDirLight( void ) 
{ 
	m_Motif.Black(); 
	m_fIntensity=0.0f; 
	m_Dir_WS.Set( -1.0f, -1.0f, 0.0f ); 
}


//-------------------------------------------------------------------------------------------------------------------
// CFWorldMesh:
//-------------------------------------------------------------------------------------------------------------------
static CFWorldMesh *_pCurrentWorldMeshToDraw;
u32 CFWorldMesh::m_nGlobalDrawnKey = 1;

//
//
//
CFWorldMesh::CFWorldMesh( void ) : CFWorldTracker( FWORLD_TRACKERTYPE_MESH ) 
{
	flinklist_InitRoot( &m_LightsRoot, CFWorldAttachedLight::GetOffsetOfLink() );
	m_nID = 0;
	m_pFcnCollPrepCallback = NULL;

	FlagAsNotDrawn(); 
	m_AmbientRGB.Black(); 
	m_DirLight.m_fIntensity=0.0f;
}


//
//
//
void CFWorldMesh::Init( FMesh_t *pMesh, BOOL bAddAttachedLights/*=TRUE*/, BOOL bAnimWhenPaused/*=FALSE*/, u32 nDrawnPartsMask/*=0xffffffff*/, u32 nCollisionPartsMask/*=0xffffffff*/, BOOL bNoMemAlloc/*=FALSE*/ ) 
{
	CFMeshInst::Init( pMesh, bAnimWhenPaused, nDrawnPartsMask, nCollisionPartsMask, bNoMemAlloc );

	if( !(m_nFlags & FMESHINST_FLAG_NOCOLLIDE) ) 
	{
		SetCollisionFlag( TRUE );
	}

	SetLineOfSightFlag( TRUE );

	if( bAddAttachedLights ) 
	{
		AddAttachedMeshLightsToWorld();
	}

	u32 i;
	for ( i = 0; i < m_pMesh->nMaterialCount; i++ ) 
	{
		if (m_pMesh->aMtl[i].nSurfaceShaderIdx == FSHADERS_oBASE_ADD_rbSREFLECT)
		{
			m_bReflect = TRUE;
		}
	}

	if (m_bReflect)
	{
		fsh_CreateSReflectTarget(this);
	}

	UpdateTracker();
}


//
//
//
void CFWorldMesh::Init( FMeshInit_t *pMeshInit, BOOL bAddAttachedLights/*=TRUE*/, u32 nDrawnPartsMask/*=0xffffffff*/, u32 nCollisionPartsMask/*=0xffffffff*/, BOOL bNoMemAlloc/*=FALSE*/ ) 
{
	CFMeshInst::Init( pMeshInit, nDrawnPartsMask, nCollisionPartsMask, bNoMemAlloc );

	if( !(m_nFlags & FMESHINST_FLAG_NOCOLLIDE) ) 
	{
		SetCollisionFlag( TRUE );
	}

	SetLineOfSightFlag( TRUE );

	if( bAddAttachedLights ) 
	{
		AddAttachedMeshLightsToWorld();
	}

	u32 i;
	for ( i = 0; i < m_pMesh->nMaterialCount; i++ ) 
	{
		if (m_pMesh->aMtl[i].nSurfaceShaderIdx == FSHADERS_oBASE_ADD_rbSREFLECT)
		{
			m_bReflect = TRUE;
		}
	}

	if (m_bReflect)
	{
		fsh_CreateSReflectTarget(this);
	}

	UpdateTracker();
}


//
//
//
void CFWorldMesh::AddAttachedMeshLightsToWorld( void ) 
{
	u32 i, nLightCount;
	CFWorldAttachedLight *pWorldAttachedLight;

	if ( !m_pMesh )
	{
		return;
	}

	// Add all attached lights to the world...
	nLightCount = m_pMesh->nLightCount;
	for( i = 0; i < nLightCount; i++ ) 
	{
		pWorldAttachedLight = fnew CFWorldAttachedLight();
		if( pWorldAttachedLight == NULL ) 
		{
			break;
		}
		pWorldAttachedLight->Init( &m_pMesh->pLightArray[i].LightInit, this );

		if( !FWorld_pWorld ) 
			pWorldAttachedLight->SetOverrideFlags( FWORLDATTACHEDLIGHT_MESH_ALLOCATED_LIGHT_IMPLICITELY );

		pWorldAttachedLight->m_Light.m_pOwnerMeshInst = this;

		AddAttachedLight( pWorldAttachedLight );
	}
}


//
// Return FALSE if the callback signalled to bail early.
//
BOOL CFWorldMesh::FindIntersectingWorldLights( FWorldIntersectingTrackerCallback_t *pCallback ) 
{
	return FindIntersectingTrackers( pCallback, FWORLD_TRACKERTYPE_LIGHT );
}


//
// Return FALSE if the callback signalled to bail early.
//
BOOL CFWorldMesh::FindIntersectingWorldMeshes( FWorldIntersectingTrackerCallback_t *pCallback ) 
{
	return FindIntersectingTrackers( pCallback, FWORLD_TRACKERTYPE_MESH );
}


//
//
//
void CFWorldMesh::RemoveFromWorld( void )
{
	CFWorldAttachedLight *pWorldAttachedLight;

	// If the object has any attached lights, remove them, too...
	for( (pWorldAttachedLight = GetFirstAttachedLight()); pWorldAttachedLight; pWorldAttachedLight = GetNextAttachedLight(pWorldAttachedLight) ) 
	{
		pWorldAttachedLight->RemoveFromWorld();
	}

	CFWorldTracker::RemoveFromWorld();
}


//
//
//
void CFWorldMesh::UpdateTracker( BOOL bCalcNewVolume/*=TRUE*/ ) 
{
	CFSphere BoundSphere_WS;
	CFWorldAttachedLight *pWorldAttachedLight;

	// First, move the object's tracker...
	m_Xfm.TransformSphereF( BoundSphere_WS, m_BoundSphere_MS );
	MoveTracker( BoundSphere_WS, bCalcNewVolume );

	// If the object has any attached lights, move them, too...
	for( (pWorldAttachedLight = GetFirstAttachedLight()); pWorldAttachedLight; pWorldAttachedLight = GetNextAttachedLight(pWorldAttachedLight) ) 
	{
		pWorldAttachedLight->UpdateTracker();
	}
}


//
//
//
void CFWorldMesh::DestroyAttachedLight( CFWorldAttachedLight *pWorldAttachedLight ) 
{
	RemoveAttachedLight( pWorldAttachedLight );
}


//
//
//
void CFWorldMesh::DestroyAllAttachedLights( void ) 
{
	CFWorldAttachedLight *pWorldAttachedLight;
	BOOL bDeleteTheLight;

	while( (pWorldAttachedLight = GetFirstAttachedLight()) ) 
	{
		bDeleteTheLight = pWorldAttachedLight->GetOverrideFlags() & FWORLDATTACHEDLIGHT_MESH_ALLOCATED_LIGHT_IMPLICITELY;

		DestroyAttachedLight( pWorldAttachedLight );

		if( bDeleteTheLight ) {
			fdelete( pWorldAttachedLight );
		}
	}
}


//
//
//
CFWorldAttachedLight* CFWorldMesh::GetAttachedLightByID( u16 nLightID ) 
{
	CFWorldAttachedLight *pWorldAttachedLight;

	pWorldAttachedLight = GetFirstAttachedLight();

	while( pWorldAttachedLight ) 
	{
		if ( pWorldAttachedLight->m_Light.m_nLightID == nLightID )
		{
			return pWorldAttachedLight;
		}

		pWorldAttachedLight = GetNextAttachedLight( pWorldAttachedLight );
	}

	return NULL;
}


//
//
//
CFWorldMesh *CFWorldMesh::FindByName( cchar *pszMeshName ) 
{
	CFWorldTracker *pTracker;
	CFWorldMesh *pWorldMesh;

	for( pTracker=CFWorldTracker::GetFirstTracker(); pTracker; pTracker=CFWorldTracker::GetNextTracker(pTracker) ) 
	{
		if( pTracker->GetType() == FWORLD_TRACKERTYPE_MESH ) 
		{
			pWorldMesh = (CFWorldMesh *)pTracker;

			if( !fclib_stricmp( pszMeshName, pWorldMesh->m_pMesh->szName ) ) 
			{
				return pWorldMesh;
			}
		}
	}

	return NULL;
}



//-------------------------------------------------------------------------------------------------------------------
// CFWorldAttachedLight:
//-------------------------------------------------------------------------------------------------------------------
void CFWorldAttachedLight::Init( const FLightInit_t *pLightInit, const CFWorldMesh *pParentMesh ) 
{
	m_nFlags = 0; //clear the flags

	FLightInit_t LightInit;

    LightInit = *pLightInit;

	// Extract and store the model space light position, radius, and direction...
	m_spInfluence_MS = pLightInit->spInfluence;
	m_mtxOrientation_MS = pLightInit->mtxOrientation;

	// Get the parent info
	m_pParentMesh = pParentMesh;
	m_nParentBoneIdx = pLightInit->nParentBoneIdx;

	// Init the world light...
	CFWorldLight::Init( &LightInit );

	UpdateOrientationAndPosition();
}


//
//
//
void CFWorldAttachedLight::UpdateOrientationAndPosition( void )
{
	// Compute and store the world space light position, radius, and direction...
	if ( m_nParentBoneIdx < 0 || !m_pParentMesh->m_pMesh->pBoneArray )
	{
		// We are attached to the root of the parent mesh
		if( !( m_nFlags & FWORLDATTACHEDLIGHT_OVERRIDE_POSITION ) ) 
		{ 
			m_pParentMesh->m_Xfm.TransformSphereF( m_Light.m_spInfluence_WS, m_spInfluence_MS );
			m_Light.SetRadius( m_pParentMesh->m_Xfm.m_fScaleF * m_spInfluence_MS.m_fRadius );
		}
		if( !( m_nFlags & FWORLDATTACHEDLIGHT_OVERRIDE_ORIENTATION ) ) 
		{ 
			m_Light.m_mtxOrientation_WS = m_pParentMesh->m_Xfm.m_MtxF.m44 * m_mtxOrientation_MS;
		}
	}
	else if ( !(m_pParentMesh->m_nFlags & FMESHINST_FLAG_NOBONES) && m_pParentMesh->GetBoneMtxPalette() )
	{
		// Parent mesh is animating so we can use the bone matrix
		CFMtx43A *pParentMtx = m_pParentMesh->GetBoneMtxPalette()[m_nParentBoneIdx];
		if( !( m_nFlags & FWORLDATTACHEDLIGHT_OVERRIDE_POSITION ) ) 
		{ 
			m_Light.m_spInfluence_WS.m_Pos = pParentMtx->m44.MultPoint( m_spInfluence_MS.m_Pos );
			f32 fMagSq = pParentMtx->m_vFront.MagSq();
			if ( fMagSq > 0.0001f )
			{
				m_Light.SetRadius( fmath_Sqrt( fMagSq ) * m_spInfluence_MS.m_fRadius );
			}
			else
			{
				m_Light.SetRadius( 0.0001f );
			}
		}
		if( !( m_nFlags & FWORLDATTACHEDLIGHT_OVERRIDE_ORIENTATION ) ) 
		{ 
			m_Light.m_mtxOrientation_WS = pParentMtx->m44 * m_mtxOrientation_MS;
		}
	}
	else
	{
		// Parent mesh is not animating
		CFMtx43A mtxTransform;
		mtxTransform.Mul( m_pParentMesh->m_Xfm.m_MtxF, m_pParentMesh->m_pMesh->pBoneArray[m_nParentBoneIdx].AtRestBoneToModelMtx );
		if( !( m_nFlags & FWORLDATTACHEDLIGHT_OVERRIDE_POSITION ) ) 
		{ 
			m_Light.m_spInfluence_WS.m_Pos = mtxTransform.m44.MultPoint( m_spInfluence_MS.m_Pos );
			f32 fMagSq = mtxTransform.m_vFront.MagSq();
			if ( fMagSq > 0.0001f )
			{
				m_Light.SetRadius( fmath_Sqrt( fMagSq ) * m_spInfluence_MS.m_fRadius );
			}
			else
			{
				m_Light.SetRadius( 0.0001f );
			}
		}
		if( !( m_nFlags & FWORLDATTACHEDLIGHT_OVERRIDE_ORIENTATION ) ) 
		{ 
			m_Light.m_mtxOrientation_WS = mtxTransform.m44 * m_mtxOrientation_MS;
		}
	}
}


//IGNORES THE OVERRIDE VALUES
void CFWorldAttachedLight::GetWSAttachedOrientation( CFMtx43 *pWSMtx )
{ 
	FASSERT( pWSMtx ); 

	if ( m_nParentBoneIdx < 0 || !m_pParentMesh->m_pMesh->pBoneArray )
	{
		*pWSMtx = m_pParentMesh->m_Xfm.m_MtxF.m44 * m_mtxOrientation_MS;
	}
	else if ( !(m_pParentMesh->m_nFlags & FMESHINST_FLAG_NOBONES) && m_pParentMesh->GetBoneMtxPalette() )
	{
		CFMtx43A *pParentMtx = m_pParentMesh->GetBoneMtxPalette()[m_nParentBoneIdx];
		*pWSMtx = pParentMtx->m44 * m_mtxOrientation_MS;
	}
	else
	{
		// Parent mesh is not animating
		CFMtx43A mtxTransform;
		mtxTransform.Mul( m_pParentMesh->m_Xfm.m_MtxF, m_pParentMesh->m_pMesh->pBoneArray[m_nParentBoneIdx].AtRestBoneToModelMtx );
		*pWSMtx = mtxTransform.m44 * m_mtxOrientation_MS;
	}
}

//IGNORES THE OVERRIDE VALUES
void CFWorldAttachedLight::GetWSAttachedPosition( CFVec3 *pMSPosVec )
{ 
	FASSERT( pMSPosVec ); 
	if ( m_nParentBoneIdx < 0 || !m_pParentMesh->m_pMesh->pBoneArray )
	{
		// We are attached to the root of the parent mesh
		if( !( m_nFlags & FWORLDATTACHEDLIGHT_OVERRIDE_POSITION ) ) { 
			*pMSPosVec = m_pParentMesh->m_Xfm.m_MtxF.m44.MultPoint( m_spInfluence_MS.m_Pos );
		}
	}
	else if ( !(m_pParentMesh->m_nFlags & FMESHINST_FLAG_NOBONES) && m_pParentMesh->GetBoneMtxPalette() )
	{
		// Parent mesh is animating so we can use the bone matrix
		CFMtx43A *pParentMtx = m_pParentMesh->GetBoneMtxPalette()[m_nParentBoneIdx];
		*pMSPosVec = pParentMtx->m44.MultPoint( m_spInfluence_MS.m_Pos );
	}
	else
	{
		// Parent mesh is not animating
		CFMtx43A mtxTransform;
		mtxTransform.Mul( m_pParentMesh->m_Xfm.m_MtxF, m_pParentMesh->m_pMesh->pBoneArray[m_nParentBoneIdx].AtRestBoneToModelMtx );
		*pMSPosVec = mtxTransform.m44.MultPoint( m_spInfluence_MS.m_Pos );
	}
}


//-------------------------------------------------------------------------------------------------------------------
// CFWorldPSGroup:
//-------------------------------------------------------------------------------------------------------------------

u32 CFWorldPSGroup::m_nGlobalDrawnKey = 1;


FCLASS_ALIGN_PREFIX class CFCollPackage 
{
public:

	f32 m_fDistToMoveSphere;
	CFVec3A m_SphereIntersectPoint;
	CFVec3A m_TriIntersectPoint;

	FCLASS_STACKMEM_ALIGN( CFCollPackage );
} FCLASS_ALIGN_SUFFIX;

