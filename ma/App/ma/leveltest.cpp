//////////////////////////////////////////////////////////////////////////////////////
// leveltest.cpp - Test level code.
//
// Author: Steve Ranck     
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
// 02/19/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#if 0
#include "fang.h"
#include "leveltest.h"
#include "gamepad.h"
#include "fpadio.h"
#include "fworld.h"
#include "fres.h"
#include "fresload.h"
#include "fanim.h"
#include "floop.h"
#include "player.h"


#define _BUILD_TITAN	FALSE


#if FANG_PLATFORM_GC
#define _TITAN_MESH_NAME		"GRMttitanGC"
#else
#define _TITAN_MESH_NAME		"GRMttitan01"
#endif
#define _TITAN_ANIM_NAME		"ARMtidle001"


CFWorldMesh *_pWorldMeshTitan;
CFAnimInst *_pAnimInst;
CFAnimCombiner *_pAnimCombiner;

#include "fmath.h"



BOOL leveltest_Load( LevelEvent_e eEvent ) {
#if _BUILD_TITAN
	FMesh_t *pMeshRes;
#endif

	// Only load the data on LEVEL_EVENT_POST_WORLD_LOAD, this was the original behavior
	if( eEvent != LEVEL_EVENT_POST_WORLD_LOAD ) {
		return TRUE;
	}

	FMeshInit_t MeshInit;

	FResFrame_t ResFrame = fres_GetFrame();

	_pAnimInst = NULL;
	_pAnimCombiner = NULL;
	_pWorldMeshTitan = NULL;

#if _BUILD_TITAN
	pMeshRes = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _TITAN_MESH_NAME );
	if( pMeshRes == NULL ) {
		DEVPRINTF( "leveltest_Load(): Could not load Titan mesh: '%s'.\n", _TITAN_MESH_NAME );
		goto _ExitLoadWithError;
	}

	_pWorldMeshTitan = fnew CFWorldMesh;
	if( _pWorldMeshTitan == NULL ) {
		DEVPRINTF( "leveltest_Load(): Not enough memory to allocate Titan world mesh.\n" );
		goto _ExitLoadWithError;
	}

	// Init the world mesh...
	MeshInit.pMesh = pMeshRes;
	MeshInit.nFlags = 0;
	MeshInit.fCullDist2 = FMATH_MAX_FLOAT;
	MeshInit.Mtx.Identity();
	MeshInit.Mtx.SetRotationY( FMATH_DEG2RAD( -90.0f ) );
	MeshInit.Mtx.m_vPos.Set( 91.82f, 5.20f, -16.55f );
	_pWorldMeshTitan->Init( &MeshInit );
	_pWorldMeshTitan->AddToWorld();
	_pWorldMeshTitan->SetCollisionFlag( TRUE );

	_pAnimInst = CFAnimInst::Load( _TITAN_ANIM_NAME );
	if( _pAnimInst == NULL ) {
		DEVPRINTF( "leveltest_Load(): Could not load Titan idle anim: '%s'.\n", _TITAN_ANIM_NAME );
		goto _ExitLoadWithError;
	}

	_pAnimCombiner = fnew CFAnimCombiner;
	if( _pAnimCombiner == NULL ) {
		DEVPRINTF( "leveltest_Load(): Not enough memory to allocate Titan anim combiner.\n" );
		goto _ExitLoadWithError;
	}

	if( !_pAnimCombiner->CreateSimple( _pAnimInst, _pWorldMeshTitan ) ) {
		DEVPRINTF( "leveltest_Load(): Could not create Titan anim combiner.\n" );
		goto _ExitLoadWithError;
	}
#endif

	return TRUE;

#if _BUILD_TITAN
	// Error:
_ExitLoadWithError:
	leveltest_Unload();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
#endif
}

void leveltest_Unload( void ) {
	fdelete( _pAnimCombiner );
	_pAnimCombiner = NULL;

	fdelete( _pAnimInst );
	_pAnimInst = NULL;

	fdelete( _pWorldMeshTitan );
	_pWorldMeshTitan = NULL;	
}

void leveltest_Work( void ) {
#if _BUILD_TITAN
	_pAnimInst->DeltaTime( FLoop_fPreviousLoopSecs );
	_pAnimCombiner->ComputeMtxPalette( FALSE );
#endif

	//_BotBlink.m_CamXfm.InitStackWithView();
}

void leveltest_Draw( void ) {
	
}

#endif