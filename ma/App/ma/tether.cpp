//////////////////////////////////////////////////////////////////////////////////////
// tether.cpp - Tether cable.
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
// 07/25/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "tether.h"
#include "flinklist.h"
#include "fmath.h"
#include "floop.h"
#include "frenderer.h"
#include "fdraw.h"
#include "fvtxpool.h"
#include "pspool.h"
#include "fworld.h"
#include "fworld_coll.h"
#include "fcoll.h"
#include "entity.h"
#include "fresload.h"
#include "fres.h"
#include "ftex.h"
#include "smoketrail.h"
#include "meshtypes.h"
#include "bot.h"
#include "botfx.h"
#include "weapon_tether.h"
#include "fforce.h"
#include "player.h"
#include "gamecam.h"
#include "hud2.h"
#include "pausescreen.h"
#include "ai/aibrainman.h"
#include "ai/aienviro.h"
#include "fsound.h"
#include "eparticlepool.h"
#include "botglitch.h"


#define _TETHER_GLOW_TEXTURE			"tem_glow02"
#define _TETHER_CABLE_TEXTURE			"TWDmcable02"
#define _DUST_PS_TEXTURE2				"tem_glow01"
#define _SMOKE_TEXTURE					"tfp1steam01"
#define _MILLOGO_TEXTURE				"MilLogo128x"
#define _CABLE_DEBRIS					"GWDmcable02"

#define _TETHER_DATA_PORT_RANGE			0.5f
#define _TETHER_THICKNESS_SCALE2		0.15f
#define _TETHER_TEX_SKEW_S_PER_FOOT2	0.03f
#define _TETHER_SPEED					150.0f

#define _FLY_SOURCE_PINCH_DIST			20.0f
#define _FLY_PROJ_PINCH_DIST			5.0f
#define _ZAP_SOURCE_PINCH_DIST			10.0f
#define _ZAP_PROJ_PINCH_DIST			5.0f
#define _ZAP_TIME						0.75f

#define _PULSE_SPEED					200.0f
#define _PULSE_WIDTH					30.0f
#define _PULSE_AMP						1.0f

#define _GLOW_WIDTH						0.7f
#define _GLOW_WIDTH_JITTER				0.2f
#define _GLOW_INTENSITY_MIN				0.4f
#define _GLOW_INTENSITY_MAX				0.7f
#define _GLOW_RED						1.0f
#define _GLOW_GREEN						1.0f
#define _GLOW_BLUE						1.0f

#define	_TIME_UNTIL_ZAP_EFFECTS_ON		0.5f

#define _INSIDE_ZAP_INTENSITY_MIN		0.2f
#define _INSIDE_ZAP_INTENSITY_MAX		1.0f
#define _INSIDE_ZAP_ALPHA				1.0f

#define _OUTSIDE_MIN_LIGHT_INTENSITY	0.1f
#define _OUTSIDE_MAX_LIGHT_INTENSITY	1.0f
#define _OUTSIDE_RED					0.6f
#define _OUTSIDE_GREEN					0.6f
#define _OUTSIDE_BLUE					1.0f
#define _OUTSIDE_ZAP_INTENSITY_MIN		0.3f
#define _OUTSIDE_ZAP_INTENSITY_MAX		1.0f
#define _OUTSIDE_ZAP_RED				0.8f
#define _OUTSIDE_ZAP_GREEN				0.8f
#define _OUTSIDE_ZAP_BLUE				1.0f
#define _OUTSIDE_ZAP_ALPHA				0.4f

#define _FIBER_MAX_UNIT_RADIUS			0.8f
#define _FIBER_COUNT_MIN				20
#define _FIBER_COUNT_MAX				30
#define _FIBER_INTENSITY_MIN			0.1f
#define _FIBER_INTENSITY_MAX			1.0f
#define _FIBER_ALPHA_MIN				0.8f
#define _FIBER_ALPHA_MAX				1.0f

#define _PS_LIFE_TIME					3.0f
#define _PS_PER_SEGMENT					10
#define _PS_RAND_VELOCITY_DELTA			0.5f
#define _PS_DIMENSION					0.2f

#define _DEBRIS_CHUNKS_PER_SEGMENT		1
#define _DEBRIS_SCALE					0.15f
#define _DEBRIS_GRAVITY_Y				-32.0f




BOOL CTether::m_bSystemInitialized;
SmokeTrailAttrib_t CTether::m_SmokeTrailAttribChunk;
SmokeTrailAttrib_t CTether::m_SmokeTrailAttribSever;
FLinkRoot_t CTether::m_LinkRoot;
CFVec3A CTether::m_aPentagonVtx[6];
f32 CTether::m_afPentagonVtxIntensity[6];
CFTexInst CTether::m_GlowTexInst;
CFTexInst CTether::m_CableTexInst;
CFTexInst CTether::m_DustTexInst;
CFTexInst CTether::m_MilLogoTexInst;
FMesh_t *CTether::m_pDebrisMesh;

CTether *CTether::m_pCallback_Tether;
CBot *CTether::m_pCallback_ClosestBot;
f32 CTether::m_fCallback_ClosestDist2;
CFVec3A CTether::m_Callback_CollPoint;




BOOL CTether::InitSystem( void ) {
	FTexDef_t *pTexDef;
	u32 i;

	FASSERT( !m_bSystemInitialized );

	// Load cable glow texture...
	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _TETHER_GLOW_TEXTURE );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CTether::InitSystem(): Could not find texture '%s'. Tether won't be drawn.\n", _TETHER_GLOW_TEXTURE );
	}
	m_GlowTexInst.SetTexDef( pTexDef );
	m_GlowTexInst.SetFlags( CFTexInst::FLAG_NONE );

	// Load cable texture...
	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _TETHER_CABLE_TEXTURE );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CTether::InitSystem(): Could not find texture '%s'. Tether won't be drawn.\n", _TETHER_CABLE_TEXTURE );
	}
	m_CableTexInst.SetTexDef( pTexDef );
	m_CableTexInst.SetFlags( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T );

	// Load dust particle texture...
	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _DUST_PS_TEXTURE2 );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CTether::InitSystem(): Could not find texture '%s'. Tether won't be drawn.\n", _DUST_PS_TEXTURE2 );
	}
	m_DustTexInst.SetTexDef( pTexDef );
	m_DustTexInst.SetFlags( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T );

	// Load Mil logo texture...
	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _MILLOGO_TEXTURE );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CTether::InitSystem(): Could not find texture '%s'. Tether won't be drawn.\n", _MILLOGO_TEXTURE );
	}
	m_MilLogoTexInst.SetTexDef( pTexDef );
	m_MilLogoTexInst.SetFlags( CFTexInst::FLAG_NONE );

	// Load debris mesh...
	m_pDebrisMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _CABLE_DEBRIS );
	if( m_pDebrisMesh == NULL ) {
		DEVPRINTF( "CTether::InitSystem(): Could not find mesh '%s'. Tether debris won't be drawn.\n", _CABLE_DEBRIS );
	}

	flinklist_InitRoot( &m_LinkRoot, (s32)FANG_OFFSETOF( CTether, m_Link ) );

	// Build pentagon corners...
	m_aPentagonVtx[0].Set(  0.0f,   -1.0f,   0.0f );
	m_aPentagonVtx[1].Set(  0.951f, -0.309f, 0.0f );
	m_aPentagonVtx[2].Set(  0.588f,  0.809f, 0.0f );
	m_aPentagonVtx[3].Set( -0.588f,  0.809f, 0.0f );
	m_aPentagonVtx[4].Set( -0.951f, -0.309f, 0.0f );
	m_aPentagonVtx[5] = m_aPentagonVtx[0];

	for( i=0; i<6; ++i ) {
		m_aPentagonVtx[i].Mul( _TETHER_THICKNESS_SCALE2 );
	}

	// Build pentagon lighting...
	for( i=0; i<6; ++i ) {
		m_afPentagonVtxIntensity[i] = 0.5f + 0.5f*fmath_Sin( (f32)i * (FMATH_2PI/5.0f) + FMATH_DEG2RAD(-20.0f) );
		m_afPentagonVtxIntensity[i] = FMATH_FPOT( m_afPentagonVtxIntensity[i], _OUTSIDE_MIN_LIGHT_INTENSITY, _OUTSIDE_MAX_LIGHT_INTENSITY );
	}

	_SetSmokeTrailAttributes();

	m_bSystemInitialized = TRUE;

	return TRUE;
}


void CTether::_SetSmokeTrailAttributes( void ) {
	m_SmokeTrailAttribChunk.nFlags = SMOKETRAIL_FLAG_NONE;
	m_SmokeTrailAttribChunk.pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _SMOKE_TEXTURE );

	m_SmokeTrailAttribChunk.fScaleMin_WS = 0.10f;
	m_SmokeTrailAttribChunk.fScaleMax_WS = 0.20f;
	m_SmokeTrailAttribChunk.fScaleSpeedMin_WS = 1.2f;
	m_SmokeTrailAttribChunk.fScaleSpeedMax_WS = 1.5f;
	m_SmokeTrailAttribChunk.fScaleAccelMin_WS = -1.0f;
	m_SmokeTrailAttribChunk.fScaleAccelMax_WS = -1.5f;

	m_SmokeTrailAttribChunk.fXRandSpread_WS = 0.2f;
	m_SmokeTrailAttribChunk.fYRandSpread_WS = 0.2f;
	m_SmokeTrailAttribChunk.fDistBetweenPuffs_WS = 0.1f;
	m_SmokeTrailAttribChunk.fDistBetweenPuffsRandSpread_WS = 0.02f;

	m_SmokeTrailAttribChunk.fYSpeedMin_WS = 0.20f;
	m_SmokeTrailAttribChunk.fYSpeedMax_WS = 0.30f;
	m_SmokeTrailAttribChunk.fYAccelMin_WS = -0.1f;
	m_SmokeTrailAttribChunk.fYAccelMax_WS = -0.3f;

	m_SmokeTrailAttribChunk.fUnitOpaqueMin_WS = 0.45f;
	m_SmokeTrailAttribChunk.fUnitOpaqueMax_WS = 0.85f;
	m_SmokeTrailAttribChunk.fUnitOpaqueSpeedMin_WS = -0.25f;
	m_SmokeTrailAttribChunk.fUnitOpaqueSpeedMax_WS = -0.75f;
	m_SmokeTrailAttribChunk.fUnitOpaqueAccelMin_WS = 0.0f;
	m_SmokeTrailAttribChunk.fUnitOpaqueAccelMax_WS = 0.0f;

	m_SmokeTrailAttribChunk.StartColorRGB.Set( _OUTSIDE_RED*0.75f, _OUTSIDE_GREEN*0.75f, _OUTSIDE_BLUE*0.75f );
	m_SmokeTrailAttribChunk.EndColorRGB.Set( _OUTSIDE_RED*0.75f, _OUTSIDE_GREEN*0.75f, _OUTSIDE_BLUE*0.75f );
	m_SmokeTrailAttribChunk.fStartColorUnitIntensityMin = 0.75f;
	m_SmokeTrailAttribChunk.fStartColorUnitIntensityMax = 1.0f;
	m_SmokeTrailAttribChunk.fEndColorUnitIntensityMin = 0.75f;
	m_SmokeTrailAttribChunk.fEndColorUnitIntensityMax = 1.0f;

	m_SmokeTrailAttribChunk.fColorUnitSliderSpeedMin = 1.0f;
	m_SmokeTrailAttribChunk.fColorUnitSliderSpeedMax = 1.5f;
	m_SmokeTrailAttribChunk.fColorUnitSliderAccelMin = 0.0f;
	m_SmokeTrailAttribChunk.fColorUnitSliderAccelMax = 0.0f;

	// Sever...
	m_SmokeTrailAttribSever.nFlags = SMOKETRAIL_FLAG_NONE;
	m_SmokeTrailAttribSever.pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _SMOKE_TEXTURE );

	m_SmokeTrailAttribSever.fScaleMin_WS = 0.15f;
	m_SmokeTrailAttribSever.fScaleMax_WS = 0.25f;
	m_SmokeTrailAttribSever.fScaleSpeedMin_WS = 1.2f;
	m_SmokeTrailAttribSever.fScaleSpeedMax_WS = 1.5f;
	m_SmokeTrailAttribSever.fScaleAccelMin_WS = -1.0f;
	m_SmokeTrailAttribSever.fScaleAccelMax_WS = -1.5f;

	m_SmokeTrailAttribSever.fXRandSpread_WS = 0.2f;
	m_SmokeTrailAttribSever.fYRandSpread_WS = 0.2f;
	m_SmokeTrailAttribSever.fDistBetweenPuffs_WS = 0.1f;
	m_SmokeTrailAttribSever.fDistBetweenPuffsRandSpread_WS = 0.02f;

	m_SmokeTrailAttribSever.fYSpeedMin_WS = 0.25f;
	m_SmokeTrailAttribSever.fYSpeedMax_WS = 0.50f;
	m_SmokeTrailAttribSever.fYAccelMin_WS = -0.2f;
	m_SmokeTrailAttribSever.fYAccelMax_WS = -0.5f;

	m_SmokeTrailAttribSever.fUnitOpaqueMin_WS = 0.45f;
	m_SmokeTrailAttribSever.fUnitOpaqueMax_WS = 0.65f;
	m_SmokeTrailAttribSever.fUnitOpaqueSpeedMin_WS = -0.75f;
	m_SmokeTrailAttribSever.fUnitOpaqueSpeedMax_WS = -1.2f;
	m_SmokeTrailAttribSever.fUnitOpaqueAccelMin_WS = 0.0f;
	m_SmokeTrailAttribSever.fUnitOpaqueAccelMax_WS = 0.0f;

	m_SmokeTrailAttribSever.StartColorRGB.Set( 0.6f, 0.6f, 1.0f );
	m_SmokeTrailAttribSever.EndColorRGB.White();
	m_SmokeTrailAttribSever.fStartColorUnitIntensityMin = 1.0f;
	m_SmokeTrailAttribSever.fStartColorUnitIntensityMax = 0.5f;
	m_SmokeTrailAttribSever.fEndColorUnitIntensityMin = 1.0f;
	m_SmokeTrailAttribSever.fEndColorUnitIntensityMax = 0.5f;

	m_SmokeTrailAttribSever.fColorUnitSliderSpeedMin = 1.5f;
	m_SmokeTrailAttribSever.fColorUnitSliderSpeedMax = 2.5f;
	m_SmokeTrailAttribSever.fColorUnitSliderAccelMin = 0.0f;
	m_SmokeTrailAttribSever.fColorUnitSliderAccelMax = 0.0f;
}


void CTether::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		m_bSystemInitialized = FALSE;
	}
}


CTether::CTether() {
	_ClearDataMembers();
}


CTether::~CTether() {
	Destroy();
}


BOOL CTether::Create( u32 nMaxVtxCount ) {
	FMeshInit_t MeshInit;
	u32 i;

	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );

	_ClearDataMembers();

	FMATH_SETBITMASK( m_nFlags, FLAG_CREATED );
	flinklist_AddTail( &m_LinkRoot, this );

	FResFrame_t ResFrame = fres_GetFrame();

	if( nMaxVtxCount < 10 ) {
		DEVPRINTF( "CTether::Create(): Must have at least 10 vertices.\n" );
		nMaxVtxCount = 10;
	}

	m_nMaxVtxCount = nMaxVtxCount;

	// Allocate position array...
	m_pPosArray_MS = fnew CFVec3A[m_nMaxVtxCount];
	if( m_pPosArray_MS == NULL ) {
		DEVPRINTF( "CTether::Create(): Not enough memory to allocate m_pPosArray_MS.\n" );
		goto _ExitWithError;
	}

	// Allocate point sprite pos array...
	m_pPSPosArray_MS = fnew CFVec3A[ (m_nMaxVtxCount-1) * _PS_PER_SEGMENT ];
	if( m_pPSPosArray_MS == NULL ) {
		DEVPRINTF( "CTether::Create(): Not enough memory to allocate m_pPSPosArray_MS.\n" );
		goto _ExitWithError;
	}

	// Allocate point sprite velocity array...
	m_pPSVelArray_MS = fnew CFVec3A[ (m_nMaxVtxCount-1) * _PS_PER_SEGMENT ];
	if( m_pPSVelArray_MS == NULL ) {
		DEVPRINTF( "CTether::Create(): Not enough memory to allocate m_pPSVelArray_MS.\n" );
		goto _ExitWithError;
	}

	// Compute how many vertices we need...
	m_nVtxPoolCount = (_FIBER_COUNT_MAX-2)*(m_nMaxVtxCount+2) + 2*(m_nMaxVtxCount+1);
	FMATH_CLAMPMIN( m_nVtxPoolCount, m_nMaxVtxCount<<1 );
	FMATH_CLAMPMIN( m_nVtxPoolCount, 12 );

	// Create debris pieces...
	m_nMaxDebrisChunckCount = (m_nMaxVtxCount-1) * _DEBRIS_CHUNKS_PER_SEGMENT;
	m_pDebrisWorldMeshArray = fnew CFWorldMesh[ m_nMaxDebrisChunckCount ];
	if( m_pDebrisWorldMeshArray == NULL ) {
		DEVPRINTF( "CTether::Create(): Not enough memory to allocate m_pDebrisWorldMeshArray.\n" );
		goto _ExitWithError;
	}

	MeshInit.pMesh = m_pDebrisMesh;
	MeshInit.nFlags = FMESHINST_FLAG_NOCOLLIDE;
	MeshInit.fCullDist = 500.0f;
	MeshInit.Mtx.Identity();

	for( i=0; i<m_nMaxDebrisChunckCount; ++i ) {
		m_pDebrisWorldMeshArray[i].Init( &MeshInit );
		m_pDebrisWorldMeshArray[i].SetLineOfSightFlag( FALSE );
		m_pDebrisWorldMeshArray[i].RemoveFromWorld();
	}

	// Allocate debris motion data...
	m_pDebrisMotionArray = fnew CFMotionSimple[ m_nMaxDebrisChunckCount ];
	if( m_pDebrisMotionArray == NULL ) {
		DEVPRINTF( "CTether::Create(): Not enough memory to allocate m_pDebrisMotionArray.\n" );
		goto _ExitWithError;
	}

	// Create dummy entities...
	m_pSourceDummyEntity = fnew CEntity;
	if( m_pSourceDummyEntity == NULL ) {
		DEVPRINTF( "CTether::Create(): Not enough memory to allocate m_pSourceDummyEntity.\n" );
		goto _ExitWithError;
	}
	if( !m_pSourceDummyEntity->Create() ) {
		DEVPRINTF( "CTether::Create(): Could not create m_pSourceDummyEntity.\n" );
		goto _ExitWithError;
	}
	m_pSourceDummyEntity->RemoveFromWorld();

	m_pTargetDummyEntity = fnew CEntity;
	if( m_pTargetDummyEntity == NULL ) {
		DEVPRINTF( "CTether::Create(): Not enough memory to allocate m_pTargetDummyEntity.\n" );
		goto _ExitWithError;
	}
	if( !m_pTargetDummyEntity->Create() ) {
		DEVPRINTF( "CTether::Create(): Could not create m_pTargetDummyEntity.\n" );
		goto _ExitWithError;
	}
	m_pTargetDummyEntity->RemoveFromWorld();

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CTether::Destroy( void ) {
	if( IsCreated() ) {
		fdelete( m_pTargetDummyEntity );
		fdelete( m_pSourceDummyEntity );
		fdelete_array( m_pDebrisMotionArray );
		fdelete_array( m_pDebrisWorldMeshArray );
		flinklist_Remove( &m_LinkRoot, this );
		_ClearDataMembers();
	}
}


void CTether::_ClearDataMembers( void ) {
	m_nMaxVtxCount = 0;
	m_nVtxCount = 0;
	m_pPosArray_MS = NULL;
	m_pPSPosArray_MS = NULL;
	m_pPSVelArray_MS = NULL;

	m_nVtxPoolCount = 0;

	m_nFlags = FLAG_NONE;
	m_nState = STATE_DESTROYED;
	fforce_NullHandle( &m_hForce );

	m_pProjEntity = NULL;
	m_pSourceDummyEntity = NULL;
	m_pTargetDummyEntity = NULL;
	m_pWeaponTether = NULL;
	m_pUserProps = NULL;

	m_nPossessMode = POSSESSMODE_NORMAL;
	m_CamInfo.m_Pos_WS.Zero();
	m_CamInfo.m_Quat_WS.Identity();
	m_DeltaCamMtx.Identity();
	m_fUnitCamAnim = 0.0f;
	m_fStartHalfFOV = 0.0f;
	m_fUnitDrawMilLogo = 0.0f;

	m_pTargetBot = NULL;
	m_pSourceBot = NULL;

	m_fPossessSpeed = 1.0f;

	m_fSpawnSparksSecs = 0.0f;

	m_fUnitGoal = 0.0f;
	m_fUnitGoalSpeed = 0.0f;
	m_fLaunchUnitRandomFactor1 = 0.0f;
	m_fLaunchUnitRandomFactor2 = 0.0f;

	m_fMaxTetherStretchDist = 0.0f;
	m_fMaxFlyDist = 0.0f;
	m_fMaxTetherLength = 0.0f;
	m_fCurrentTetherLength = 0.0f;
	m_fDistanceBetweenVtx = 0.0f;

	m_fUnitSourcePinchDisable = 0.0f;
	m_fVtxUngulateAngle = 0.0f;

	m_fUnitTighten = 0.0f;
	m_fTightenOOVtxCountMinusOne = 1.0f;

	m_fTimeUntilZapOn = 0.0f;
	m_fZapPulseDist = 0.0f;
	m_fZappingTimeRemaining = 0.0f;

	m_fUnitDustRemaining = 0.0f;
	m_nPSDrawCount = 0;

	m_pDebrisWorldMeshArray = NULL;
	m_pDebrisMotionArray = NULL;
	m_nMaxDebrisChunckCount = 0;
	m_nDebrisChunkDrawCount = 0;

	m_ModelMtx.Identity();
	m_Velocity_WS.Zero();
}


// Assumes the FDraw renderer is currently active.
// Assumes the game's main perspective camera is
// set up. Assumes there are no model Xfms on the
// stack.
//
// Does not preserve the current FDraw state.
// Does not preserve the current viewport.
// Will preserve the Xfm stack.
// Will preserve the frenderer fog state.
void CTether::DrawAll( void ) {
	CTether *pTether;
	BOOL bSetState;

	FASSERT( m_bSystemInitialized );

	if( m_CableTexInst.GetTexDef() == NULL ) {
		// No texture. Cannot draw...
		return;
	}

	// Draw tether cable...
	bSetState = FALSE;
	for( pTether=(CTether *)flinklist_GetHead( &m_LinkRoot ); pTether; pTether=(CTether *)flinklist_GetNext( &m_LinkRoot, pTether ) ) {
		if( pTether->m_nState != STATE_DESTROYED ) {
			if( !bSetState ) {
				frenderer_SetDefaultState();
				bSetState = TRUE;
			}

			pTether->_Draw();
		}
	}

	// Draw dust effect...
	bSetState = FALSE;
	for( pTether=(CTether *)flinklist_GetHead( &m_LinkRoot ); pTether; pTether=(CTether *)flinklist_GetNext( &m_LinkRoot, pTether ) ) {
		if( pTether->m_nFlags & FLAG_DEBRIS_DUST ) {
			if( !bSetState ) {
				frenderer_Push( FRENDERER_PSPRITE, NULL );
				bSetState = TRUE;
			}

			pTether->_DrawDust();
		}
	}

	if( bSetState ) {
		frenderer_Pop();
	}
}


void CTether::_Draw( void ) {
	FDrawVtx_t *pVtxArray;
	CFXfm Xfm;

	if( m_nVtxCount < 2 ) {
		// Nothing to draw...
		return;
	}

	pVtxArray = fvtxpool_GetArray( m_nVtxPoolCount );
	if( pVtxArray == NULL ) {
		// Couldn't get a pool of FDrawVtx_t's.
		// We'll try again next frame...
		return;
	}

	Xfm.BuildFromMtx( m_ModelMtx );
	Xfm.PushModel();

	_DrawInside( pVtxArray );
	_DrawFibers( pVtxArray );
	_DrawOutside( pVtxArray );
	_DrawCableGlow( pVtxArray );

	CFXfm::PopModel();
	fvtxpool_ReturnArray( pVtxArray );
}


void CTether::_DrawCableGlow( FDrawVtx_t *pVtxArray ) {
	u32 nSegCount, nSegIndex;
	FDrawVtx_t *pDrawVtx;
	CFVec3A *pVtxPos;
	f32 fVtxDist, fUnitGlowIntensity, fOOCableLength, fAlpha;
	CFColorRGBA ColorRGBA;

	if( m_fZapPulseDist == 0.0f ) {
		return;
	}

	if( m_GlowTexInst.GetTexDef() == NULL ) {
		return;
	}

	fdraw_SetCullDir( FDRAW_CULLDIR_NONE );
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_SetTexture( &m_GlowTexInst );

	fUnitGlowIntensity = fmath_RandomFloatRange( _GLOW_INTENSITY_MIN, _GLOW_INTENSITY_MAX );

	ColorRGBA.Set( _GLOW_RED, _GLOW_GREEN, _GLOW_BLUE, fUnitGlowIntensity );
	pDrawVtx = pVtxArray;

	pDrawVtx->Pos_MS.x = (_GLOW_WIDTH * -0.5f) + m_pPosArray_MS->x - _GLOW_WIDTH_JITTER*fmath_RandomFloat();
	pDrawVtx->Pos_MS.y = m_pPosArray_MS->y;
	pDrawVtx->Pos_MS.z = m_pPosArray_MS->z;
	pDrawVtx->ST.x = 0.0f;
	pDrawVtx->ST.y = 0.0f;
	pDrawVtx->ColorRGBA.ColorRGB = ColorRGBA.ColorRGB;
	pDrawVtx->ColorRGBA.fAlpha = 0.0f;
	++pDrawVtx;

	pDrawVtx->Pos_MS.x = (_GLOW_WIDTH * 0.5f) + m_pPosArray_MS->x + _GLOW_WIDTH_JITTER*fmath_RandomFloat();
	pDrawVtx->Pos_MS.y = m_pPosArray_MS->y;
	pDrawVtx->Pos_MS.z = m_pPosArray_MS->z;
	pDrawVtx->ST.x = 1.0f;
	pDrawVtx->ST.y = 0.0f;
	pDrawVtx->ColorRGBA.ColorRGB = ColorRGBA.ColorRGB;
	pDrawVtx->ColorRGBA.fAlpha = 0.0f;
	++pDrawVtx;

	// Draw all segments...
	fOOCableLength = fmath_Inv( m_fCurrentTetherLength );
	fVtxDist = m_fCurrentTetherLength - m_fDistanceBetweenVtx;
	nSegCount = m_nVtxCount - 1;
	for( nSegIndex=0, pVtxPos=&m_pPosArray_MS[1]; nSegIndex<nSegCount; ++nSegIndex, ++pVtxPos, fVtxDist-=m_fDistanceBetweenVtx ) {
		fAlpha = (m_fZapPulseDist - fVtxDist) * (1.0f / _PULSE_WIDTH);
		FMATH_CLAMP( fAlpha, 0.0f, 1.0f );

		pDrawVtx->Pos_MS.x = (_GLOW_WIDTH * -0.5f) + pVtxPos->x - _GLOW_WIDTH_JITTER*fmath_RandomFloat();;
		pDrawVtx->Pos_MS.y = pVtxPos->y;
		pDrawVtx->Pos_MS.z = pVtxPos->z;
		pDrawVtx->ST.x = 0.0f;
		pDrawVtx->ST.y = 0.0f;
		pDrawVtx->ColorRGBA.ColorRGB = ColorRGBA.ColorRGB;
		pDrawVtx->ColorRGBA.fAlpha = ColorRGBA.fAlpha * fAlpha;
		++pDrawVtx;

		pDrawVtx->Pos_MS.x = (_GLOW_WIDTH * 0.5f) + pVtxPos->x + _GLOW_WIDTH_JITTER*fmath_RandomFloat();;
		pDrawVtx->Pos_MS.y = pVtxPos->y;
		pDrawVtx->Pos_MS.z = pVtxPos->z;
		pDrawVtx->ST.x = 1.0f;
		pDrawVtx->ST.y = 0.0f;
		pDrawVtx->ColorRGBA.ColorRGB = ColorRGBA.ColorRGB;
		pDrawVtx->ColorRGBA.fAlpha = ColorRGBA.fAlpha * fAlpha;
		++pDrawVtx;
	}

	pDrawVtx[-2].ColorRGBA.fAlpha = 0.0f;
	pDrawVtx[-1].ColorRGBA.fAlpha = 0.0f;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtxArray, m_nVtxCount << 1 );
}


void CTether::_DrawInside( FDrawVtx_t *pVtxArray ) {
	u32 i, nSegCount, nSegIndex;
	FDrawVtx_t *pDrawVtx;
	CFVec3A *pPentagonVtx, *pVtxPos;
	f32 fS, fStartS, fDeltaSegS, fUnitIntensity;
	CFColorRGBA ColorRGBA;

	fdraw_SetCullDir( FDRAW_CULLDIR_CW );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_SetTexture( &m_CableTexInst );

	// Init texture coordinate info...
	fDeltaSegS = _TETHER_TEX_SKEW_S_PER_FOOT2 * m_fDistanceBetweenVtx;
	fStartS = 0.0f;

	// Build vertices near projectile...
	if( m_fZapPulseDist > 0.0f ) {
		fdraw_Depth_EnableWriting( FALSE );
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

		fUnitIntensity = fmath_RandomFloatRange( _INSIDE_ZAP_INTENSITY_MIN, _INSIDE_ZAP_INTENSITY_MAX );
		ColorRGBA.Set( fUnitIntensity, fUnitIntensity, fUnitIntensity, _INSIDE_ZAP_ALPHA );
	} else {
		fdraw_Depth_EnableWriting( TRUE );
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_SRC );

		ColorRGBA.Set( 1.0f, 1.0f, 1.0f, 1.0f );
	}

	for( i=0, pDrawVtx=pVtxArray, pPentagonVtx=m_aPentagonVtx, fS=0.0f; i<6; ++i, pDrawVtx+=2, ++pPentagonVtx, fS+=0.2f ) {
		pDrawVtx->Pos_MS.x = pPentagonVtx->x + m_pPosArray_MS->x;
		pDrawVtx->Pos_MS.y = pPentagonVtx->y + m_pPosArray_MS->y;
		pDrawVtx->Pos_MS.z = m_pPosArray_MS->z;
		pDrawVtx->ST.x = fS;
		pDrawVtx->ST.y = 0.0f;
		pDrawVtx->ColorRGBA = ColorRGBA;
	}

	// Draw all segments...
	nSegCount = m_nVtxCount - 1;
	for( nSegIndex=0, pVtxPos=&m_pPosArray_MS[1]; nSegIndex<nSegCount; ++nSegIndex, ++pVtxPos ) {
		fStartS += fDeltaSegS;

		if( m_fZapPulseDist > 0.0f ) {
			fUnitIntensity = fmath_RandomFloatRange( _INSIDE_ZAP_INTENSITY_MIN, _INSIDE_ZAP_INTENSITY_MAX );
			ColorRGBA.Set( fUnitIntensity, fUnitIntensity, fUnitIntensity, _INSIDE_ZAP_ALPHA );
		} else {
			ColorRGBA.Set( 1.0f, 1.0f, 1.0f, 1.0f );
		}

		if( nSegIndex ) {
			for( i=0; i<12; i+=2 ) {
				pVtxArray[i] = pVtxArray[i+1];
			}
		}

		for( i=0, pDrawVtx=&pVtxArray[1], pPentagonVtx=m_aPentagonVtx, fS=fStartS; i<6; ++i, pDrawVtx+=2, ++pPentagonVtx, fS+=0.2f ) {
			pDrawVtx->Pos_MS.x = pPentagonVtx->x + pVtxPos->x;
			pDrawVtx->Pos_MS.y = pPentagonVtx->y + pVtxPos->y;
			pDrawVtx->Pos_MS.z = pVtxPos->z;
			pDrawVtx->ST.x = fS;
			pDrawVtx->ST.y = 0.0f;
			pDrawVtx->ColorRGBA = ColorRGBA;
		}

		fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtxArray, 12 );
	}
}


void CTether::_DrawOutside( FDrawVtx_t *pVtxArray ) {
	u32 i, nSegCount, nSegIndex;
	FDrawVtx_t *pDrawVtx;
	CFVec3A *pPentagonVtx, *pVtxPos;
	f32 fS, fStartS, fDeltaSegS, fUnitIntensity;
	CFColorRGBA ColorRGBA;

	fdraw_SetCullDir( FDRAW_CULLDIR_CCW );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_SetTexture( &m_CableTexInst );

	// Init texture coordinate info...
	fDeltaSegS = _TETHER_TEX_SKEW_S_PER_FOOT2 * m_fDistanceBetweenVtx;
	fStartS = 0.0f;

	// Build vertices near projectile...
	if( m_fZapPulseDist > 0.0f ) {
		fdraw_Depth_EnableWriting( FALSE );
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

		fUnitIntensity = fmath_RandomFloatRange( _OUTSIDE_ZAP_INTENSITY_MIN, _OUTSIDE_ZAP_INTENSITY_MAX );
		ColorRGBA.Set( _OUTSIDE_ZAP_RED*fUnitIntensity, _OUTSIDE_ZAP_GREEN*fUnitIntensity, _OUTSIDE_ZAP_BLUE*fUnitIntensity, _OUTSIDE_ZAP_ALPHA );
	} else {
		fdraw_Depth_EnableWriting( TRUE );
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_SRC );
	}

	for( i=0, pDrawVtx=pVtxArray, pPentagonVtx=m_aPentagonVtx, fS=0.0f; i<6; ++i, pDrawVtx+=2, ++pPentagonVtx, fS+=0.2f ) {
		if( m_fZapPulseDist > 0.0f ) {
			// Store vertex color...
			pDrawVtx->ColorRGBA = ColorRGBA;
		} else {
			// Compute vertex lighting...
			fUnitIntensity = m_afPentagonVtxIntensity[i];
			pDrawVtx->ColorRGBA.Set( fUnitIntensity, fUnitIntensity, fUnitIntensity, 1.0f );
		}

		pDrawVtx->Pos_MS.x = pPentagonVtx->x + m_pPosArray_MS->x;
		pDrawVtx->Pos_MS.y = pPentagonVtx->y + m_pPosArray_MS->y;
		pDrawVtx->Pos_MS.z = m_pPosArray_MS->z;
		pDrawVtx->ST.x = fS;
		pDrawVtx->ST.y = 0.0f;
	}

	// Draw all segments...
	nSegCount = m_nVtxCount - 1;
	for( nSegIndex=0, pVtxPos=&m_pPosArray_MS[1]; nSegIndex<nSegCount; ++nSegIndex, ++pVtxPos ) {
		fStartS += fDeltaSegS;

		if( m_fZapPulseDist > 0.0f ) {
			fUnitIntensity = fmath_RandomFloatRange( _OUTSIDE_ZAP_INTENSITY_MIN, _OUTSIDE_ZAP_INTENSITY_MAX );
			ColorRGBA.Set( _OUTSIDE_ZAP_RED*fUnitIntensity, _OUTSIDE_ZAP_GREEN*fUnitIntensity, _OUTSIDE_ZAP_BLUE*fUnitIntensity, _OUTSIDE_ZAP_ALPHA );
		}

		if( nSegIndex ) {
			for( i=0; i<12; i+=2 ) {
				pVtxArray[i] = pVtxArray[i+1];
			}
		}

		for( i=0, pDrawVtx=&pVtxArray[1], pPentagonVtx=m_aPentagonVtx, fS=fStartS; i<6; ++i, pDrawVtx+=2, ++pPentagonVtx, fS+=0.2f ) {
			if( m_fZapPulseDist > 0.0f ) {
				// Store vertex color...
				pDrawVtx->ColorRGBA = ColorRGBA;
			} else {
				// Compute vertex lighting...
				fUnitIntensity = m_afPentagonVtxIntensity[i];
				pDrawVtx->ColorRGBA.Set( fUnitIntensity, fUnitIntensity, fUnitIntensity, 1.0f );
			}

			pDrawVtx->Pos_MS.x = pPentagonVtx->x + pVtxPos->x;
			pDrawVtx->Pos_MS.y = pPentagonVtx->y + pVtxPos->y;
			pDrawVtx->Pos_MS.z = pVtxPos->z;
			pDrawVtx->ST.x = fS;
			pDrawVtx->ST.y = 0.0f;
		}

		fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtxArray, 12 );
	}

	// Draw endcap near source...
	fdraw_SetCullDir( FDRAW_CULLDIR_CW );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_SetTexture( NULL );

	pVtxPos = &m_pPosArray_MS[nSegCount];
	pDrawVtx = pVtxArray;

	pDrawVtx->Pos_MS.x = m_aPentagonVtx[0].x + pVtxPos->x;
	pDrawVtx->Pos_MS.y = m_aPentagonVtx[0].y + pVtxPos->y;
	pDrawVtx->Pos_MS.z = pVtxPos->z;
	++pDrawVtx;

	pDrawVtx->Pos_MS.x = m_aPentagonVtx[1].x + pVtxPos->x;
	pDrawVtx->Pos_MS.y = m_aPentagonVtx[1].y + pVtxPos->y;
	pDrawVtx->Pos_MS.z = pVtxPos->z;
	++pDrawVtx;

	pDrawVtx->Pos_MS.x = m_aPentagonVtx[4].x + pVtxPos->x;
	pDrawVtx->Pos_MS.y = m_aPentagonVtx[4].y + pVtxPos->y;
	pDrawVtx->Pos_MS.z = pVtxPos->z;
	++pDrawVtx;

	pDrawVtx->Pos_MS.x = m_aPentagonVtx[2].x + pVtxPos->x;
	pDrawVtx->Pos_MS.y = m_aPentagonVtx[2].y + pVtxPos->y;
	pDrawVtx->Pos_MS.z = pVtxPos->z;
	++pDrawVtx;

	pDrawVtx->Pos_MS.x = m_aPentagonVtx[3].x + pVtxPos->x;
	pDrawVtx->Pos_MS.y = m_aPentagonVtx[3].y + pVtxPos->y;
	pDrawVtx->Pos_MS.z = pVtxPos->z;
	++pDrawVtx;

	for( i=0, pDrawVtx=pVtxArray; i<5; ++i, ++pDrawVtx ) {
		fUnitIntensity = fmath_RandomFloatRange( _OUTSIDE_ZAP_INTENSITY_MIN, _OUTSIDE_ZAP_INTENSITY_MAX );
		ColorRGBA.Set( _OUTSIDE_ZAP_RED*fUnitIntensity, _OUTSIDE_ZAP_GREEN*fUnitIntensity, _OUTSIDE_ZAP_BLUE*fUnitIntensity, _OUTSIDE_ZAP_ALPHA );

		pDrawVtx->ColorRGBA = ColorRGBA;
		pDrawVtx->ST.Zero();
	}

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtxArray, 5 );
}


void CTether::_DrawFibers( FDrawVtx_t *pVtxArray ) {
	u32 nFiberCount, nFiberIndex, nSegCount, nSegIndex;
	f32 fSin, fCos, fRandomRadius, fRandomIntensity, fRandomAlpha;
	CFVec3A *pVtxPos;
	FDrawVtx_t *pDrawVtx, *pPrevDrawVtx;
	CFColorRGBA ColorRGBA;

	if( m_fZapPulseDist == 0.0f ) {
		return;
	}

	fdraw_SetCullDir( FDRAW_CULLDIR_NONE );
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_SetTexture( NULL );

	nFiberCount = fmath_RandomRange( _FIBER_COUNT_MIN, _FIBER_COUNT_MAX );

	pDrawVtx = pVtxArray;
	nSegCount = m_nVtxCount - 1;

	for( nFiberIndex=0; nFiberIndex<nFiberCount; ++nFiberIndex ) {
		fmath_SinCos( FMATH_2PI * fmath_RandomFloat(), &fSin, &fCos );
		fRandomIntensity = fmath_RandomFloatRange( _FIBER_INTENSITY_MIN, _FIBER_INTENSITY_MAX );
		fRandomAlpha = fmath_RandomFloatRange( _FIBER_ALPHA_MIN, _FIBER_ALPHA_MAX );
		ColorRGBA.Set( fRandomIntensity, fRandomIntensity, fRandomIntensity, fRandomAlpha );

		if( nFiberIndex ) {
			// Leave room for the dummy vertex...
			++pDrawVtx;
		}

		// Set up the vertex at projectile end of tether...
		fRandomRadius = (_FIBER_MAX_UNIT_RADIUS * _TETHER_THICKNESS_SCALE2 * 0.808f) * fmath_RandomFloat();
		pDrawVtx->Pos_MS.x = m_pPosArray_MS->x + fRandomRadius * fSin;
		pDrawVtx->Pos_MS.y = m_pPosArray_MS->y + fRandomRadius * fCos;
		pDrawVtx->Pos_MS.z = m_pPosArray_MS->z;
		pDrawVtx->ST.Zero();
		pDrawVtx->ColorRGBA = ColorRGBA;

		// Set up dummy vertex...
		if( nFiberIndex ) {
			pPrevDrawVtx = pDrawVtx - 1;
			pPrevDrawVtx->Pos_MS.x = pDrawVtx->Pos_MS.x;
			pPrevDrawVtx->Pos_MS.y = pDrawVtx->Pos_MS.y;
			pPrevDrawVtx->Pos_MS.z = pDrawVtx->Pos_MS.z;
			pPrevDrawVtx->ST.x = 0.0f;
			pPrevDrawVtx->ST.y = 0.0f;
			pPrevDrawVtx->ColorRGBA.TransparentBlack();
		}

		++pDrawVtx;

		// Set up remaining vertices...
		for( nSegIndex=0, pVtxPos=&m_pPosArray_MS[1]; nSegIndex<nSegCount; ++nSegIndex, ++pVtxPos ) {
			fRandomRadius = (_FIBER_MAX_UNIT_RADIUS * _TETHER_THICKNESS_SCALE2 * 0.808f) * fmath_RandomFloat();
			pDrawVtx->Pos_MS.x = pVtxPos->x + fRandomRadius * fSin;
			pDrawVtx->Pos_MS.y = pVtxPos->y + fRandomRadius * fCos;
			pDrawVtx->Pos_MS.z = pVtxPos->z;
			pDrawVtx->ST.Zero();
			pDrawVtx->ColorRGBA = ColorRGBA;
			++pDrawVtx;
		}

		// Set up dummy vertex...
		if( nFiberIndex < (nFiberCount-1) ) {
			pPrevDrawVtx = pDrawVtx - 1;
			pDrawVtx->Pos_MS.x = pPrevDrawVtx->Pos_MS.x;
			pDrawVtx->Pos_MS.y = pPrevDrawVtx->Pos_MS.y;
			pDrawVtx->Pos_MS.z = pPrevDrawVtx->Pos_MS.z;
			pDrawVtx->ST.x = 0.0f;
			pDrawVtx->ST.y = 0.0f;
			pDrawVtx->ColorRGBA.TransparentBlack();
			++pDrawVtx;
		}
	}

	FASSERT( nFiberCount >= 2 );

	fdraw_PrimList( FDRAW_PRIMTYPE_LINESTRIP, pVtxArray, (nFiberCount-2)*(m_nVtxCount+2) + 2*(m_nVtxCount+1) );
}


void CTether::_DrawDust( void ) {
	FPSprite_t *pPSArray, *pPS;
	CFVec3A *pPSVtx;
	u32 i;
	f32 fUnitAlpha;
	BOOL bSparkle;

	pPSArray = pspool_GetArray( m_nPSDrawCount );
	if( pPSArray == NULL ) {
		// Couldn't get a pool of FPSprite_t's.
		// We'll try again next frame...
		return;
	}

	fUnitAlpha = FMATH_FPOT( m_fUnitDustRemaining, 0.1f, 1.0f );

	for( i=0, pPS=pPSArray, pPSVtx=m_pPSPosArray_MS; i<m_nPSDrawCount; ++i, ++pPS, ++pPSVtx ) {
		pPS->Point_MS = pPSVtx->v3;
		pPS->fDim_MS = _PS_DIMENSION;

		bSparkle = fmath_RandomChance( 0.1f*(1.0f-m_fUnitDustRemaining) );

		if( !bSparkle ) {
			pPS->ColorRGBA.Set(
				fmath_RandomFloatRange( 0.5f, 1.0f ) * _OUTSIDE_RED,
				fmath_RandomFloatRange( 0.5f, 1.0f ) * _OUTSIDE_GREEN,
				fmath_RandomFloatRange( 0.5f, 1.0f ) * _OUTSIDE_BLUE,
				m_fUnitDustRemaining * 0.5f
			);
		} else {
			pPS->ColorRGBA.Set( 1.0f, 1.0f, 1.0f, fUnitAlpha );
		}
	}

	m_PSGroup.m_pBase = pPSArray;
	m_PSGroup.Render();

	pspool_ReturnArray( pPSArray );
}


void CTether::_DrawMilLogoOverlay( u32 nPlayerIndex, void *pUser ) {
	CTether *pTether = (CTether *)pUser;
	if( pTether->m_pWeaponTether == NULL ) {
		return;
	}

	FASSERT( pTether->m_pWeaponTether->GetOwner()->m_nPossessionPlayerIndex >= 0 );
	CPlayer *pPlayer = &Player_aPlayer[ pTether->m_pWeaponTether->GetOwner()->m_nPossessionPlayerIndex ];

	FViewport_t *pViewport = pPlayer->m_pViewportOrtho3D;

	CFColorRGBA ColorRGBA;
	FDrawVtx_t aVtx[4];
	f32 fZ, fTexOffset;
	u32 i;

	ColorRGBA.Set( 1.0f, 1.0f, 1.0f, pTether->m_fUnitDrawMilLogo * pTether->m_fUnitDrawMilLogo * 0.75f );

	for( i=0; i<4; ++i ) {
		aVtx[i].ColorRGBA = ColorRGBA;
	}

	fZ = 0.5f * (pViewport->fNearZ + pViewport->fFarZ);

	aVtx[0].Pos_MS.Set( -pViewport->HalfRes.x,  pViewport->HalfRes.y, fZ );
	aVtx[1].Pos_MS.Set(  pViewport->HalfRes.x,  pViewport->HalfRes.y, fZ );
	aVtx[2].Pos_MS.Set( -pViewport->HalfRes.x, -pViewport->HalfRes.y, fZ );
	aVtx[3].Pos_MS.Set(  pViewport->HalfRes.x, -pViewport->HalfRes.y, fZ );

	fTexOffset = FMATH_FPOT( pTether->m_fUnitDrawMilLogo, 1.4f, 0.8f );

	aVtx[0].ST.Set( 0.5f - fTexOffset, 0.5f - fTexOffset );
	aVtx[1].ST.Set( 0.5f + fTexOffset, 0.5f - fTexOffset );
	aVtx[2].ST.Set( 0.5f - fTexOffset, 0.5f + fTexOffset );
	aVtx[3].ST.Set( 0.5f + fTexOffset, 0.5f + fTexOffset );

	fviewport_SetActive( pViewport );
	CFXfm::InitStack();

	frenderer_Push( FRENDERER_DRAW, NULL );

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_SetTexture( &m_MilLogoTexInst );

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aVtx, 4 );

	frenderer_Pop();
}


// Set pTargetEntity to NULL if an entity is not to be targeted.
void CTether::Launch( CWeaponTether *pWeaponTether, cchar *pszFireEmitterBoneName, CBot *pTargetBot, const CFVec3A *pTargetPos_WS,
					  f32 fMaxTetherLength, f32 fMaxTetherStretchDist, f32 fMaxFlyDist, f32 fPossessSpeed, const UserProps_t *pUserProps ) {
	FASSERT( IsCreated() );

	f32 fSourceToTargetDist, fMag2;
	CFMtx43A Mtx;

	// Set up source...
	m_pSourceDummyEntity->DetachFromParent();
	m_pSourceDummyEntity->Attach_UnitMtxToParent_PS_NewScale_WS( pWeaponTether, pszFireEmitterBoneName );
	m_pSourceDummyEntity->AddToWorld();

	m_ModelMtx.m_vPos = m_pSourceDummyEntity->MtxToWorld()->m_vPos;

	// Set up targeting...
	m_pTargetBot = pTargetBot;

	m_pTargetDummyEntity->DetachFromParent();
	CFMtx43A::m_Xlat.m_vPos = *pTargetPos_WS;
	m_pTargetDummyEntity->Relocate_RotXlatFromUnitMtx_WS( &CFMtx43A::m_Xlat );

	if( m_pTargetBot ) {
		// Attach dummy entity to target entity's data port...
		m_pTargetBot->DataPort_AttachEntity_WS( m_pTargetDummyEntity );
	}
	m_pTargetDummyEntity->AddToWorld();

	// Compute initial velocity...
	m_Velocity_WS.Sub( m_pTargetDummyEntity->MtxToWorld()->m_vPos, m_ModelMtx.m_vPos );
	fMag2 = m_Velocity_WS.MagSq();
	if( fMag2 > 0.001f ) {
		m_Velocity_WS.Mul( _TETHER_SPEED * fmath_InvSqrt(fMag2) );
	} else {
		m_Velocity_WS.Zero();
	}

	fSourceToTargetDist = m_ModelMtx.m_vPos.Dist( m_pTargetDummyEntity->MtxToWorld()->m_vPos );
	FMATH_CLAMPMIN( fSourceToTargetDist, 1.0f );

	m_pWeaponTether = pWeaponTether;
	m_pUserProps = pUserProps;
	m_fPossessSpeed = fPossessSpeed;

	m_nPossessMode = POSSESSMODE_NORMAL;
	m_fUnitCamAnim = 0.0f;
	m_fUnitDrawMilLogo = 0.0f;

	m_fUnitGoal = 0.0f;
	m_fUnitGoalSpeed = fmath_Div( _TETHER_SPEED, fSourceToTargetDist );
	m_fMaxTetherStretchDist = fMaxTetherStretchDist;
	m_fMaxFlyDist = fMaxFlyDist;
	m_fMaxTetherLength = fMaxTetherLength;
	m_fCurrentTetherLength = 0.0f;
	m_fUnitSourcePinchDisable = 0.0f;
	m_nVtxCount = 0;

	m_fDistanceBetweenVtx = fmath_Div( fMaxTetherLength, (f32)m_nMaxVtxCount );
	m_fVtxUngulateAngle = 0.0f;
	m_fUnitTighten = 0.0f;
	m_fTimeUntilZapOn = 0.0f;
	m_fZapPulseDist = 0.0f;
	m_fZappingTimeRemaining = 0.0f;
	m_fLaunchUnitRandomFactor1 = fmath_RandomFloat();
	m_fLaunchUnitRandomFactor2 = fmath_RandomFloat();

	m_fSpawnSparksSecs = 0.0f;

	FMATH_CLEARBITMASK( m_nFlags, FLAG_DETACHED | FLAG_WHITESAT_VIEW | FLAG_DRAWING_MIL_LOGO );

	m_nState = STATE_FLYING;
	fforce_Kill( &m_hForce );
}


void CTether::Sever( void ) {
	FASSERT( IsCreated() );

	if( m_nState == STATE_FLYING ) {
		if( !(m_nFlags & FLAG_DETACHED) ) {
			FMATH_SETBITMASK( m_nFlags, FLAG_DETACHED );

			if( m_pWeaponTether->GetSafeOwnerPlayerIndex() >= 0 ) {
				// 2D sound...
				CFSoundGroup::PlaySound( m_pUserProps->pSoundGroupDetach );
			} else {
				// 3D sound...
				CFSoundGroup::PlaySound( m_pUserProps->pSoundGroupDetach, FALSE, &m_pSourceDummyEntity->MtxToWorld()->m_vPos );
			}

			// Spawn sparks...
			fparticle_SpawnEmitter(
				m_pUserProps->hParticleSeverSparks,
				m_pSourceDummyEntity->MtxToWorld()->m_vPos.v3,
				&m_ModelMtx.m_vFront.v3,
				1.0f
			);

			// Make a small smoke trail...
			smoketrail_SpawnMuzzlePuff(
				&m_SmokeTrailAttribSever,
				&m_pSourceDummyEntity->MtxToWorld()->m_vPos,
				&m_pSourceDummyEntity->MtxToWorld()->m_vFront,
				3.0f
			);

			fforce_Kill( &m_hForce );

			m_fUnitSourcePinchDisable = 0.0f;
			m_fMaxTetherLength = m_fCurrentTetherLength;
		}
	} else if( m_nState == STATE_ZAPPING ) {
		Kill();
	}
}


void CTether::Kill( BOOL bSupressEffects ) {
	if( m_nFlags & FLAG_CONTACT ) {
		_EmergencyDisconnect();
		FMATH_CLEARBITMASK( m_nFlags, FLAG_CONTACT );
	}

	if( m_nState == STATE_DESTROYED ) {
		return;
	}

	if( !bSupressEffects && (m_nVtxCount >= 2) ) {
		// Start effects...

		u32 i, j;
		f32 fUnit, fMag2;
		CFVec3A *pOrigVtx, *pPSVtx, *pStartVtx, *pEndVtx, PSSourceVtx, BoundSphereOrigin_WS, UnitVelocity;
		CFMtx43A Mtx;
		CFQuatA Quat;
		CFMotionSimple *pMotion;
		CFWorldMesh *pWorldMesh;
		SmokeTrailHandle_t hSmokeTrail;

		CFSoundGroup::PlaySound( m_pUserProps->pSoundGroupShatter, FALSE, &m_ModelMtx.m_vPos, -1, TRUE );

		FMATH_SETBITMASK( m_nFlags, FLAG_DEBRIS_DUST | FLAG_DEBRIS_CHUNKS );

		// Compute point sprite initial positions...
		hSmokeTrail = smoketrail_GetFromFreePoolAndSetAttributes( &m_SmokeTrailAttribChunk );
		for( i=1, pOrigVtx=m_pPosArray_MS, pPSVtx=m_pPSPosArray_MS; i<m_nVtxCount; ++i, ++pOrigVtx, pPSVtx+=_PS_PER_SEGMENT ) {
			m_ModelMtx.MulPoint( *pPSVtx, *pOrigVtx );
			smoketrail_Puff( hSmokeTrail, pPSVtx );
		}
		m_ModelMtx.MulPoint( PSSourceVtx, *pOrigVtx );
		if( hSmokeTrail != SMOKETRAIL_NULLHANDLE ) {
			smoketrail_Puff( hSmokeTrail, &PSSourceVtx );
			smoketrail_ReturnToFreePool( hSmokeTrail, TRUE );
		}

		fMag2 = m_Velocity_WS.MagSq();
		if( fMag2 > 0.0001f ) {
			UnitVelocity.Mul( m_Velocity_WS, fmath_InvSqrt( fMag2 ) );
		} else {
			UnitVelocity.Zero();
		}

		// Remove all old debris from the world...
		for( i=0; i<m_nDebrisChunkDrawCount; ++i ) {
			m_pDebrisWorldMeshArray[i].RemoveFromWorld();
		}

		pPSVtx = m_pPSPosArray_MS;
		pMotion = m_pDebrisMotionArray;
		pWorldMesh = m_pDebrisWorldMeshArray;
		for( i=2; i<=m_nVtxCount; ++i ) {
			pStartVtx = pPSVtx;

			if( i < m_nVtxCount ) {
				pEndVtx = &pPSVtx[_PS_PER_SEGMENT];
			} else {
				pEndVtx = &PSSourceVtx;
			}

			// Create point sprites...
			for( j=1, ++pPSVtx; j<_PS_PER_SEGMENT; ++j, ++pPSVtx ) {
				fUnit = (f32)j * (1.0f / _PS_PER_SEGMENT);
				pPSVtx->Lerp( fUnit, *pStartVtx, *pEndVtx );
				pPSVtx->x += fmath_RandomFloatRange( -_TETHER_THICKNESS_SCALE2, _TETHER_THICKNESS_SCALE2 );
				pPSVtx->y += fmath_RandomFloatRange( -_TETHER_THICKNESS_SCALE2, _TETHER_THICKNESS_SCALE2 );
				pPSVtx->z += fmath_RandomFloatRange( -_TETHER_THICKNESS_SCALE2, _TETHER_THICKNESS_SCALE2 );
			}

			// Compute unit axis...
			Mtx.m_vFront.Sub( *pEndVtx, *pStartVtx );
			fMag2 = Mtx.m_vFront.MagSq();
			if( fMag2 > 0.001f ) {
				Mtx.m_vFront.Mul( fmath_InvSqrt(fMag2) );
				Mtx.m_vRight.UnitCrossYWithVec( Mtx.m_vFront );
				Mtx.m_vUp.Cross( Mtx.m_vFront, Mtx.m_vRight );
				Quat.BuildQuat( Mtx );

				// Create debris...
				for( j=0; j<_DEBRIS_CHUNKS_PER_SEGMENT; ++j, ++pMotion, ++pWorldMesh ) {
					fUnit = (f32)j * (1.0f / _DEBRIS_CHUNKS_PER_SEGMENT);

					pMotion->m_OrientQuat = Quat;
					pMotion->m_Pos.Lerp( fUnit, *pStartVtx, *pEndVtx );

					pMotion->m_Vel.Mul( m_Velocity_WS, fmath_RandomFloatRange( 0.1f, 0.3f ) );
					pMotion->m_Vel.y = fmath_RandomFloatRange( 0.0f, 20.0f );

					pMotion->m_RotUnitAxis.x = fmath_RandomFloatRange( 0.1f, 1.0f );
					pMotion->m_RotUnitAxis.y = fmath_RandomFloatRange( 0.1f, 1.0f );
					pMotion->m_RotUnitAxis.z = fmath_RandomFloatRange( 0.1f, 1.0f );
					pMotion->m_RotUnitAxis.Unitize();
					pMotion->m_fAngularSpeed = fmath_RandomFloatRange( -FMATH_2PI * 2.0f, FMATH_2PI * 2.0f );

					pMotion->m_OrientQuat.BuildMtx( Mtx );
					Mtx.m_vPos = pMotion->m_Pos;
					pWorldMesh->m_Xfm.BuildFromMtx( Mtx, _DEBRIS_SCALE );
					pWorldMesh->UpdateTracker();
				}
			}
		}

		m_nDebrisChunkDrawCount = (m_nVtxCount - 1) * _DEBRIS_CHUNKS_PER_SEGMENT;
		m_nPSDrawCount = (m_nVtxCount - 1) * _PS_PER_SEGMENT;

		// Compute point sprite initial velocities...
		for( i=0, pPSVtx=m_pPSVelArray_MS; i<m_nPSDrawCount; ++i, ++pPSVtx ) {
			pPSVtx->x = fmath_RandomFloatRange( -_PS_RAND_VELOCITY_DELTA, _PS_RAND_VELOCITY_DELTA );
			pPSVtx->y = 0.0f;
			pPSVtx->z = fmath_RandomFloatRange( -_PS_RAND_VELOCITY_DELTA, _PS_RAND_VELOCITY_DELTA );
		}

		m_fUnitDustRemaining = 1.0f;

		// Build point sprite group...
		BoundSphereOrigin_WS.Lerp( 0.5f, PSSourceVtx, m_pPSPosArray_MS[ m_nPSDrawCount-1 ] );
		m_PSGroup.m_Xfm.Identity();
		m_PSGroup.m_BoundSphere_MS.m_Pos = BoundSphereOrigin_WS.v3;
		m_PSGroup.m_BoundSphere_MS.m_fRadius = 0.5f * m_fCurrentTetherLength;
		m_PSGroup.m_fCullDist = 500.0f;
		m_PSGroup.m_nPSFlags = FPSPRITE_FLAG_NONE;
		m_PSGroup.m_nBlend = FPSPRITE_BLEND_MODULATE;
		m_PSGroup.m_TexInst = m_DustTexInst;
		m_PSGroup.m_nMaxCount = m_nPSDrawCount;
		m_PSGroup.m_pBase = NULL;
		m_PSGroup.m_nRenderCount = m_nPSDrawCount;
		m_PSGroup.m_nRenderStartIndex = 0;

		m_Velocity_WS.Zero();
	}

	m_nState = STATE_DESTROYED;
	FMATH_CLEARBITMASK( m_nFlags, FLAG_DETACHED );
	fforce_Kill( &m_hForce );

	m_pSourceDummyEntity->DetachFromParent();
	m_pTargetDummyEntity->DetachFromParent();
	m_pSourceDummyEntity->RemoveFromWorld();
	m_pTargetDummyEntity->RemoveFromWorld();

	m_pWeaponTether = NULL;
	m_nVtxCount = 0;

	m_fUnitGoal = 0.0f;
	m_fUnitGoalSpeed = 0.0f;

	m_fMaxTetherStretchDist = 0.0f;
	m_fMaxFlyDist = 0.0f;
	m_fMaxTetherLength = 0.0f;
	m_fCurrentTetherLength = 0.0f;
	m_fDistanceBetweenVtx = 0.0f;

	m_fUnitSourcePinchDisable = 0.0f;
	m_fVtxUngulateAngle = 0.0f;

	m_fUnitTighten = 0.0f;
	m_fTightenOOVtxCountMinusOne = 0.0f;

	m_fTimeUntilZapOn = 0.0f;
	m_fZapPulseDist = 0.0f;
	m_fZappingTimeRemaining = 0.0f;

	m_ModelMtx.Identity();
	m_Velocity_WS.Zero();
}


void CTether::_Zap( void ) {
	FASSERT( IsCreated() );

	if( m_nState == STATE_FLYING ) {
		if( !(m_nFlags & FLAG_DETACHED) ) {
			m_nState = STATE_ZAPPING;
			fforce_Kill( &m_hForce );

			m_fUnitTighten = 0.0f;
			m_Velocity_WS.Zero();

			// Target is now wherever the projectile is...
			if( m_pTargetBot ) {
				if (m_pTargetBot->IsSleeping())
				{
					m_pTargetBot->WakeUp();
				}
				m_pTargetDummyEntity->Relocate_RotXlatFromUnitMtx_WS( &CFMtx43A::m_IdentityMtx, FALSE );
				m_pTargetBot->DataPort_AttachEntity_PS( m_pTargetDummyEntity );
			} else {
				m_pTargetDummyEntity->Relocate_RotXlatFromUnitMtx_WS( &m_ModelMtx, FALSE );
			}

			m_fUnitGoal = 1.0f;

			// Reset the zap stuff...
			m_fTimeUntilZapOn = _TIME_UNTIL_ZAP_EFFECTS_ON;
			m_fZapPulseDist = 0.0f;
			m_fZappingTimeRemaining = 0.0f;

			// Compute the distance between vertices...
			if( m_nVtxCount >= 2 ) {
				m_fTightenOOVtxCountMinusOne = fmath_Inv( (f32)(m_nVtxCount - 1) );
			} else {
				m_nVtxCount = 0;
				m_fUnitTighten = 1.0f;
				m_fTightenOOVtxCountMinusOne = 1.0f;
			}
		}
	}
}


void CTether::WorkAll( void ) {
	CTether *pTether;

	FASSERT( m_bSystemInitialized );

	if( FLoop_bGamePaused ) {
		return;
	}

	for( pTether=(CTether *)flinklist_GetHead( &m_LinkRoot ); pTether; pTether=(CTether *)flinklist_GetNext( &m_LinkRoot, pTether ) ) {
		switch( pTether->m_nState ) {
		case STATE_DESTROYED:
			if( pTether->m_nFlags & FLAG_CONTACT ) {
				// Somehow this tether got destroyed while in the process of possessing a bot. Disconnect it...
				pTether->_EmergencyDisconnect();
				FMATH_CLEARBITMASK( pTether->m_nFlags, FLAG_CONTACT );
			}

			break;

		case STATE_FLYING:
			pTether->_Work_Flying();
			break;

		case STATE_ZAPPING:
			pTether->_Work_Zapping();
			break;
		}

		if( pTether->m_nFlags & FLAG_DEBRIS_DUST ) {
			pTether->_Work_DebrisDust();
		}

		if( pTether->m_nFlags & FLAG_DEBRIS_CHUNKS ) {
			pTether->_Work_DebrisChunks();
		}

		if( pTether->m_nPossessMode != POSSESSMODE_NORMAL ) {
			switch( pTether->m_nPossessMode ) {
			case POSSESSMODE_FLY_ALONG_CABLE:
				if( pTether->m_nState != STATE_DESTROYED ) {
					pTether->_Work_PossessMode_FlyAlongCable();
				}
				break;

			case POSSESSMODE_FADE_IN_BEHIND_BOT:
				pTether->_Work_PossessMode_FadeInBehindBot();
				break;

			case POSSESSMODE_POWERING_UP_BOT:
				pTether->_Work_PossessMode_PoweringUpBot();
				break;

			default:
				FASSERT_NOW;
			}
		}
	}
}


void CTether::_Work_Flying( void ) {
	f32 fSourceToTargetDist, fDistFromSourceToProj, fNormDistBetweenVtx, fUnitGoal, fArcDriver, fVtxDist, fProjArcOffsetY;
	f32 fRandomPhaseX, fRandomPhaseY, fSourceUnitAmp, fProjUnitAmp, fUnitAmp, fDistFromVtxToSource, fUngulateDispX;
	f32 fScaledSourceToTargetDist, fScaledSourceToTargetDist2;
	u32 i, nArcVtxCount, nVtxCountMinusOne;
	BOOL bEnterZapMode;
	FCollImpact_t CollImpact;
	CFVec3A m_PrevProjPos_WS;

	bEnterZapMode = FALSE;

	// Update m_fUnitGoal...
	m_fUnitGoal += FLoop_fPreviousLoopSecs * m_fUnitGoalSpeed;

	// Update ungulation angle...
	m_fVtxUngulateAngle += FLoop_fPreviousLoopSecs * (1.5f * FMATH_2PI);

	// Update source pinch slider...
	if( m_nFlags & FLAG_DETACHED ) {
		if( m_fUnitSourcePinchDisable < 1.0f ) {
			m_fUnitSourcePinchDisable += FLoop_fPreviousLoopSecs * 2.0f;
			FMATH_CLAMPMAX( m_fUnitSourcePinchDisable, 1.0f );
		}
	}

	// Compute the unit vector from source to target, and
	// its length...
	m_ModelMtx.m_vFront.Sub( m_pTargetDummyEntity->MtxToWorld()->m_vPos, m_pSourceDummyEntity->MtxToWorld()->m_vPos );
	fSourceToTargetDist = m_ModelMtx.m_vFront.Mag();
	if( fSourceToTargetDist < 0.01f ) {
		// Target is too close to source...
		Kill();
		return;
	}

	m_ModelMtx.m_vFront.Div( fSourceToTargetDist );
	fScaledSourceToTargetDist = fSourceToTargetDist * (1.0f / 50.0f);
	fScaledSourceToTargetDist2 = fScaledSourceToTargetDist * fScaledSourceToTargetDist;

	// Compute the normalized distance between vertices...
	fNormDistBetweenVtx = fmath_Div( m_fDistanceBetweenVtx, fSourceToTargetDist );

	// Record previous values...
	m_PrevProjPos_WS = m_ModelMtx.m_vPos;

	// Compute world space position of projectile...
	m_ModelMtx.m_vPos.Lerp( m_fUnitGoal, m_pSourceDummyEntity->MtxToWorld()->m_vPos, m_pTargetDummyEntity->MtxToWorld()->m_vPos );

	// Compute projectile velocity...
	m_Velocity_WS.Sub( m_ModelMtx.m_vPos, m_PrevProjPos_WS );
	m_Velocity_WS.Mul( FLoop_fPreviousLoopOOSecs );

	// Compute the arc Y offset for the projectile...
	fArcDriver = (m_fUnitGoal * 2.0f - 1.0f) * fScaledSourceToTargetDist;
	fProjArcOffsetY = fScaledSourceToTargetDist2 - fArcDriver*fArcDriver;

	// Adjust matrix by the arc Y offset so that the origin is the projectile...
	m_ModelMtx.m_vPos.y += fProjArcOffsetY;

	// Perform collision...
	_BuildTrackerSkipList();
	if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &m_PrevProjPos_WS, &m_ModelMtx.m_vPos, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList, TRUE, NULL, ~ENTITY_BIT_SHIELD, FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES ) ) {
		// Our tether projectile hit something...

		if( m_pWeaponTether ) {
			if( (!CollImpact.pTag || !( ((CFWorldMesh*) CollImpact.pTag)->m_nUser ==MESHTYPES_ENTITY && (((CEntity*) ((CFWorldMesh*) CollImpact.pTag)->m_pUser)->TypeBits() & ENTITY_BIT_BOT)) ) &&
				m_pWeaponTether->GetOwner()->m_nPossessionPlayerIndex>-1 && m_pWeaponTether->m_fSecsUntilNextSound<=0.0f ) {
				AIEnviro_AddSound( CollImpact.ImpactPoint, AIEnviro_fLaserImpactAudibleRange*4.0f, 0.3f, AISOUNDTYPE_BOTFOOD, 0, m_pWeaponTether );
				m_pWeaponTether->m_fSecsUntilNextSound = 0.5f;
			}
		}

		if( m_nFlags & FLAG_DETACHED ) {
			// Tether is detached, so always kill...

			m_ModelMtx.m_vPos.Set( CollImpact.ImpactPoint );

			Kill();

			return;
		} else {
			// Tether is not detached, so ask callback what to do...

			if( _HandleTetherProjectileCollision( &CollImpact ) ) {
				// Zap mode...

				m_ModelMtx.m_vPos = *m_pTargetBot->DataPort_GetPos();

				// reserve the dataport here so no one else can zap it at the same time
				m_pTargetBot->DataPort_Reserve( TRUE );
				bEnterZapMode = TRUE;

				FMATH_SETBITMASK( m_nFlags, FLAG_CONTACT );
			} else {
				// Kill...

				m_ModelMtx.m_vPos.Set( CollImpact.ImpactPoint );

				Kill();

				return;
			}
		}
	}

	// Compute the distance from the source to the projectile...
	fDistFromSourceToProj = m_fUnitGoal * fSourceToTargetDist;

	// See if it's gone too far...
	if( fDistFromSourceToProj > m_fMaxFlyDist ) {
		Kill();
		return;
	}

	// Complete building the transformation matrix...
	m_ModelMtx.m_vRight.UnitCrossYWithVec( m_ModelMtx.m_vFront );
	m_ModelMtx.m_vUp.Cross( m_ModelMtx.m_vFront, m_ModelMtx.m_vRight );

	// Compute tether length...
	if( fDistFromSourceToProj < m_fMaxTetherLength ) {
		// Tether is still attached to its source...
		m_fCurrentTetherLength = fDistFromSourceToProj;
	} else {
		if( !(m_nFlags & FLAG_DETACHED) ) {
			// Tether has detached from its source...
			m_fCurrentTetherLength = m_fMaxTetherLength;

			m_pSourceDummyEntity->DetachFromParent();

			Sever();

			if( bEnterZapMode ) {
				// Collision and disconnect in the same frame...
				Kill();
				return;
			}
		}
	}

	// Recompute unit goal if necessary...
	if( bEnterZapMode ) {
		m_fUnitGoal = fmath_Div( fDistFromSourceToProj, fSourceToTargetDist );
		fArcDriver = (m_fUnitGoal * 2.0f - 1.0f) * fScaledSourceToTargetDist;
		fProjArcOffsetY = fScaledSourceToTargetDist2 - fArcDriver*fArcDriver;
	}

	// Compute the number of vertices we need...
	m_nVtxCount = (u32)fmath_Div( m_fCurrentTetherLength + 0.95f*m_fDistanceBetweenVtx, m_fDistanceBetweenVtx ) + 1;
	if( m_nVtxCount < 2 ) {
		Kill();
		return;
	}
	FMATH_CLAMPMAX( m_nVtxCount, m_nMaxVtxCount );
	nVtxCountMinusOne = m_nVtxCount - 1;

	// Add in slack distortion...
	fRandomPhaseX = FMATH_2PI * m_fLaunchUnitRandomFactor1;
	fRandomPhaseY = FMATH_2PI * m_fLaunchUnitRandomFactor2;

	if( m_nFlags & FLAG_DETACHED ) {
		nArcVtxCount = m_nVtxCount;
	} else {
		CFMtx43A InvMtx;

		m_pPosArray_MS[nVtxCountMinusOne].x = 0.0f;
		m_pPosArray_MS[nVtxCountMinusOne].y = -fProjArcOffsetY;
		m_pPosArray_MS[nVtxCountMinusOne].z = 0.0f;

		InvMtx.ReceiveAffineInverse( m_ModelMtx, FALSE );
		InvMtx.MulDir( m_pPosArray_MS[nVtxCountMinusOne] );
		m_pPosArray_MS[nVtxCountMinusOne].z -= m_fCurrentTetherLength;

		nArcVtxCount = nVtxCountMinusOne;
	}

	for( i=0, fUnitGoal=m_fUnitGoal, fVtxDist=0.0f; i<nArcVtxCount; ++i, fUnitGoal-=fNormDistBetweenVtx, fVtxDist+=m_fDistanceBetweenVtx ) {
		// Pinch near projectile...
		fProjUnitAmp = 1.0f;
		if( fVtxDist < _FLY_PROJ_PINCH_DIST ) {
			// Vertex is within projectile pinch distance...
			fProjUnitAmp = fVtxDist * (1.0f / _FLY_PROJ_PINCH_DIST);
		}

		// Pinch near source...
		fSourceUnitAmp = 1.0f;
		if( m_fUnitSourcePinchDisable < 1.0f ) {
			fDistFromVtxToSource = m_fCurrentTetherLength - fVtxDist;
			if( fDistFromVtxToSource < _FLY_SOURCE_PINCH_DIST ) {
				// Vertex is within source pinch distance...
				fSourceUnitAmp = m_fUnitSourcePinchDisable + fDistFromVtxToSource * (1.0f / _FLY_SOURCE_PINCH_DIST);
				FMATH_CLAMPMAX( fSourceUnitAmp, 1.0f );
			}
		}

		// Compute amplitude...
		fUnitAmp = fProjUnitAmp * fSourceUnitAmp * fSourceUnitAmp;

		// Add in arc curvature...
		fArcDriver = (fUnitGoal * 2.0f - 1.0f) * fScaledSourceToTargetDist;
		m_pPosArray_MS[i].y = fScaledSourceToTargetDist2 - fArcDriver*fArcDriver - fProjArcOffsetY;

		// Add in waves...
		fUngulateDispX =  0.5f * fmath_Sin( m_fVtxUngulateAngle + fVtxDist * (1.0f / 2.0f) );
		m_pPosArray_MS[i].x = fUnitAmp * (fmath_Sin( fVtxDist * (1.0f / 4.0f) + fRandomPhaseX ) + fUngulateDispX);
		m_pPosArray_MS[i].y += fUnitAmp * fmath_Sin( fVtxDist * (1.0f / 7.0f) + fRandomPhaseY );
		m_pPosArray_MS[i].z = -fVtxDist;
	}

	if( bEnterZapMode ) {
		_Zap();
	}

	// Spawn sparks...
	if( !(m_nFlags & FLAG_DETACHED) ) {
		m_fSpawnSparksSecs -= FLoop_fPreviousLoopSecs;
		if( m_fSpawnSparksSecs <= 0.0f ) {
			m_fSpawnSparksSecs = 0.06f;

			fparticle_SpawnEmitter(
				m_pUserProps->hParticleSeverSparks,
				m_pSourceDummyEntity->MtxToWorld()->m_vPos.v3,
				&m_pSourceDummyEntity->MtxToWorld()->m_vFront.v3,
				0.0f
			);
		}
	}
}


void CTether::_Work_Zapping( void ) {
	f32 fVtxDist, fOneMinusUnitTighten, fVibeSign, fMaxZapPulseDist, fUnitVal, fSin, fPulseAmp, fDot;
	f32 fSourceToTargetDist, fProjUnitAmp, fSourceUnitAmp, fDistFromVtxToSource, fOppositeVtxDist;
	u32 i;
	BOOL bCanStartZapping;
	CBot *pBot;

	// Update tightening factor...
	if( m_fUnitTighten < 1.0f ) {
		m_fUnitTighten += FLoop_fPreviousLoopSecs * 2.0f * m_fPossessSpeed;
		FMATH_CLAMPMAX( m_fUnitTighten, 1.0f );
	}

	// Update zap effect timer...
	if( m_fTimeUntilZapOn > 0.0f ) {
		m_fTimeUntilZapOn -= FLoop_fPreviousLoopSecs * m_fPossessSpeed;
		if( m_fTimeUntilZapOn <= 0.0f ) {
			m_fTimeUntilZapOn = 0.0f;

			CFSoundGroup::PlaySound( m_pUserProps->pSoundGroupZapSurge );

			if( m_pWeaponTether->IsOwnedByPlayer()) {
				fforce_Play(
					Player_aPlayer[ m_pWeaponTether->GetOwner()->m_nPossessionPlayerIndex ].m_nControllerIndex,
					FFORCE_EFFECT_ENERGY_PULSE,
					&m_hForce
				);
			}
		}
	} else {
		fMaxZapPulseDist = m_fCurrentTetherLength + _PULSE_WIDTH;

		if( m_fZapPulseDist < fMaxZapPulseDist ) {
			bCanStartZapping = (m_fZapPulseDist < m_fCurrentTetherLength);

			m_fZapPulseDist += _PULSE_SPEED * FLoop_fPreviousLoopSecs * m_fPossessSpeed;

			if( m_fZapPulseDist >= fMaxZapPulseDist ) {
				m_fZapPulseDist = fMaxZapPulseDist;
			}

			if( bCanStartZapping && (m_fZappingTimeRemaining == 0.0f) && (m_fZapPulseDist >= m_fCurrentTetherLength) ) {
				m_fZappingTimeRemaining = _ZAP_TIME + FLoop_fPreviousLoopSecs * m_fPossessSpeed;

				if( m_pTargetBot ) {
					CFVec3A DataPortUnitNormal_WS;
					m_pTargetBot->DataPort_ComputeUnitNormal( &DataPortUnitNormal_WS );

					fparticle_SpawnEmitter(
						m_pUserProps->hParticleDataPortBurst,
						m_pTargetBot->DataPort_GetPos()->v3,
						&DataPortUnitNormal_WS.v3,
						1.0f
					);

					m_pTargetBot->DataPort_Shock( TRUE );
				}
			}
		}

		if( m_fZappingTimeRemaining > 0.0f ) {
			m_fZappingTimeRemaining -= FLoop_fPreviousLoopSecs * m_fPossessSpeed;
			if( m_fZappingTimeRemaining <= 0.0f ) {
				m_fZappingTimeRemaining = 0.0f;

				fforce_Kill( &m_hForce );

				if( m_pTargetBot ) {
					m_pTargetBot->DataPort_Shock( FALSE );
					m_pTargetBot->Power( FALSE, 0.0f, m_fPossessSpeed );

					// Get the player index and his camera
					s32 nPlayerIndex = m_pWeaponTether->GetOwner()->m_nPossessionPlayerIndex;

					CFSoundGroup::PlaySound( m_pUserProps->pSoundGroupFlyDownCable );

					CFCamera* pGCam = fcamera_GetCameraByIndex(PLAYER_CAM(nPlayerIndex));

					// Save off original camera FOV...
					pGCam->GetFOV( &m_fStartHalfFOV );

					// Compute delta camera matrix...
					CFMtx43A InvMount;

					InvMount.ReceiveAffineInverse( *m_pWeaponTether->GetOwner()->MtxToWorld(), FALSE );
					m_DeltaCamMtx.Mul( InvMount, pGCam->GetFinalXfm()->m_MtxR );

					// Initialize our new camera's position and orientation to the same as the old camera's...
					m_CamInfo.m_Pos_WS = pGCam->GetFinalXfm()->m_MtxR.m_vPos;
					m_CamInfo.m_Quat_WS.BuildQuat( pGCam->GetFinalXfm()->m_MtxR );

					gamecam_SwitchPlayerToSimpleCamera( PLAYER_CAM(nPlayerIndex), &m_CamInfo );

					// Initialize our new camera's FOV to the same as the old camera's...
					pGCam->SetFOV( m_fStartHalfFOV );

					// Start camera fly mode...
					m_nPossessMode = POSSESSMODE_FLY_ALONG_CABLE;
					m_fUnitCamAnim = 0.0f;
					m_fUnitDrawMilLogo = 0.0f;
					FMATH_CLEARBITMASK( m_nFlags, FLAG_WHITESAT_VIEW | FLAG_DRAWING_MIL_LOGO );

					// Don't allow pause screen or weapon select here...
					CPauseScreen::SetEnabled( FALSE );
					CHud2::GetHudForPlayer(nPlayerIndex)->SetWSEnable( FALSE );
				} else {
					// This is here for development purposes...
					Kill();
					return;
				}
			}
		}
	}

	// Update the projectile position...
	m_ModelMtx.m_vPos = m_pTargetDummyEntity->MtxToWorld()->m_vPos;

	// Compute the unit vector from source to target, and
	// its length...
	m_ModelMtx.m_vFront.Sub( m_pTargetDummyEntity->MtxToWorld()->m_vPos, m_pSourceDummyEntity->MtxToWorld()->m_vPos );
	fSourceToTargetDist = m_ModelMtx.m_vFront.Mag();
	if( fSourceToTargetDist < 0.01f ) {
		// Target is too close to source...
		Kill();
		return;
	}
	m_ModelMtx.m_vFront.Div( fSourceToTargetDist );

	// See if tether is stretched too far...
	if( fSourceToTargetDist > m_fMaxTetherStretchDist ) {
		// Tether stretched too far...
		Kill();
		return;
	}

	// Make sure source angle isn't too great...
	pBot = m_pWeaponTether->GetOwner();
	if( pBot ) {
		fDot = pBot->MtxToWorld()->m_vFront.Dot( m_ModelMtx.m_vFront );
		if( fDot <= 0.0f ) {
			Kill();
			return;
		}
	}

	// Make sure target angle isn't too great...
	if( m_pTargetBot ) {
		CFVec3A DataPortUnitNormal_WS;
		m_pTargetBot->DataPort_ComputeUnitNormal( &DataPortUnitNormal_WS );
		fDot = DataPortUnitNormal_WS.Dot( m_ModelMtx.m_vFront );
		if( fDot >= 0.0f ) {
			Kill();
			return;
		}
	}

	// Compute the distance between vertices...
	m_fDistanceBetweenVtx = fSourceToTargetDist * m_fTightenOOVtxCountMinusOne;

	// Complete building the transformation matrix...
	m_ModelMtx.m_vRight.UnitCrossYWithVec( m_ModelMtx.m_vFront );
	m_ModelMtx.m_vUp.Cross( m_ModelMtx.m_vFront, m_ModelMtx.m_vRight );

	// Fill vertices...
	fOneMinusUnitTighten = 1.0f - m_fUnitTighten;
	fVibeSign = fOneMinusUnitTighten * ((FVid_nFrameCounter & 1) ? -0.3f : 0.3f);
	for( i=0, fVtxDist=0.0f; i<m_nVtxCount; ++i, fVtxDist+=m_fDistanceBetweenVtx ) {
		// Add vibration...
		fUnitVal = fmath_Sin( FMATH_PI * (f32)i * m_fTightenOOVtxCountMinusOne );
		m_pPosArray_MS[i].x = fVibeSign * fUnitVal;
		m_pPosArray_MS[i].y = 0.0f;
		m_pPosArray_MS[i].z = -fVtxDist;

		// Add zap pulse...
		if( m_fZapPulseDist > 0.0f ) {
			fOppositeVtxDist = m_fCurrentTetherLength - fVtxDist;
			fUnitVal = (m_fZapPulseDist - fOppositeVtxDist) * (1.0f / _PULSE_WIDTH);
			FMATH_CLAMP( fUnitVal, 0.0f, 1.0f );

			fSin = fmath_Sin( fUnitVal * FMATH_PI );
			fPulseAmp = _PULSE_AMP * fmath_Sin( fUnitVal * FMATH_2PI ) * fSin * fSin;

			// Pinch pulse near projectile...
			fProjUnitAmp = 1.0f;
			if( fVtxDist < _ZAP_PROJ_PINCH_DIST ) {
				// Vertex is within projectile pinch distance...
				fProjUnitAmp = fVtxDist * (1.0f / _ZAP_PROJ_PINCH_DIST);
			}

			// Pinch pulse near source...
			fSourceUnitAmp = 1.0f;
			if( m_fUnitSourcePinchDisable < 1.0f ) {
				fDistFromVtxToSource = m_fCurrentTetherLength - fVtxDist;
				if( fDistFromVtxToSource < _ZAP_SOURCE_PINCH_DIST ) {
					// Vertex is within source pinch distance...
					fSourceUnitAmp = m_fUnitSourcePinchDisable + fDistFromVtxToSource * (1.0f / _ZAP_SOURCE_PINCH_DIST);
					FMATH_CLAMPMAX( fSourceUnitAmp, 1.0f );
				}
			}

			m_pPosArray_MS[i].x += fPulseAmp * fProjUnitAmp * fSourceUnitAmp;
		}
	}
}


void CTether::_Work_DebrisDust( void ) {
	u32 i;
	CFVec3A *pPSPos, *pPSVel, DeltaPos_WS;

	m_fUnitDustRemaining -= FLoop_fPreviousLoopSecs * (1.0f / _PS_LIFE_TIME);
	if( m_fUnitDustRemaining <= 0.0f ) {
		// We're done with the dust...
		m_fUnitDustRemaining = 0.0f;
		FMATH_CLEARBITMASK( m_nFlags, FLAG_DEBRIS_DUST );
		return;
	}

	for( i=0, pPSPos=m_pPSPosArray_MS, pPSVel=m_pPSVelArray_MS; i<m_nPSDrawCount; ++i, ++pPSPos, ++pPSVel ) {
		// Update position...
		DeltaPos_WS.Mul( *pPSVel, FLoop_fPreviousLoopSecs );
		pPSPos->Add( DeltaPos_WS );

		if( pPSVel->y > 0.0f ) {
			pPSVel->y -= 1000.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( pPSVel->y, 0.0f );
		} else {
			pPSVel->y += -2.0f * FLoop_fPreviousLoopSecs;
		}
	}
}


void CTether::_Work_DebrisChunks( void ) {
	u32 i, nAliveChunkCount;
	CFMotionSimple *pMotion;
	CFWorldMesh *pWorldMesh;
	FCollImpact_t CollImpact;
	CFVec3A PrevPos_WS;
	CFMtx43A Mtx;

	_BuildTrackerSkipList();

	nAliveChunkCount = 0;

	for( i=0, pMotion=m_pDebrisMotionArray, pWorldMesh=m_pDebrisWorldMeshArray; i<m_nDebrisChunkDrawCount; ++i, ++pMotion, ++pWorldMesh ) {
		if( pWorldMesh->IsAddedToWorld() ) {
			PrevPos_WS = pMotion->m_Pos;

			pMotion->m_Vel.y += _DEBRIS_GRAVITY_Y * FLoop_fPreviousLoopSecs;

			if( pMotion->m_Vel.y < -100.0f ) {
				pWorldMesh->RemoveFromWorld();
				_SpawnChunkImpactEffects( &pMotion->m_Pos, NULL );
			} else {
				pMotion->Simulate();

				if( !fworld_FindClosestImpactPointToRayStart( &CollImpact, &PrevPos_WS, &pMotion->m_Pos, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList, TRUE, NULL, -1, FCOLL_MASK_COLLIDE_WITH_DEBRIS ) ) {
					++nAliveChunkCount;

					pMotion->m_OrientQuat.BuildMtx( Mtx );
					Mtx.m_vPos = pMotion->m_Pos;
					pWorldMesh->m_Xfm.BuildFromMtx( Mtx, _DEBRIS_SCALE );
					pWorldMesh->UpdateTracker();
				} else {
					pWorldMesh->RemoveFromWorld();
					_SpawnChunkImpactEffects( &CollImpact.ImpactPoint, &CollImpact.UnitFaceNormal );
				}
			}
		}
	}

	if( nAliveChunkCount == 0 ) {
		FMATH_CLEARBITMASK( m_nFlags, FLAG_DEBRIS_CHUNKS );
		return;
	}
}


void CTether::_SpawnChunkImpactEffects( const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS ) {
	CFVec3A UnitDir_WS;

	if( pUnitDir_WS ) {
		UnitDir_WS = *pUnitDir_WS;
	} else {
		UnitDir_WS.Set( 0.0f, -1.0f, 0.0f );
	}

	fparticle_SpawnEmitter(
		m_pUserProps->hParticleChunkPuff,
		pPos_WS->v3,
		&UnitDir_WS.v3,
		1.0f
	);

	if( m_pUserProps->pDebrisGroupChunk ) {
		CFDebrisSpawner DebrisSpawner;
		DebrisSpawner.InitToDefaults();

		DebrisSpawner.m_Mtx.m_vPos = *pPos_WS;
		DebrisSpawner.m_Mtx.m_vZ = UnitDir_WS;
		DebrisSpawner.m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_POINT;
		DebrisSpawner.m_pDebrisGroup = m_pUserProps->pDebrisGroupChunk;
		DebrisSpawner.m_fMinSpeed = 10.0f;
		DebrisSpawner.m_fMaxSpeed = 20.0f;
		DebrisSpawner.m_fUnitDirSpread = 0.5f;
		DebrisSpawner.m_nMinDebrisCount = 2;
		DebrisSpawner.m_nMaxDebrisCount = 4;

		CGColl::SpawnDebris( &DebrisSpawner );
	}

	CFSoundGroup::PlaySound( m_pUserProps->pSoundGroupChunkImpact, FALSE, pPos_WS );
}


// Returns TRUE if the tether projectile hit a data port, or
// FALSE if it hit something else.
BOOL CTether::_HandleTetherProjectileCollision( FCollImpact_t *pCollImpact ) {
	CFSphere Sphere;
	CFWorldUser UserTracker;
	BOOL bHitDataPort;

	bHitDataPort = FALSE;

	_BuildTrackerSkipList();

	// Build user tracker...
	Sphere.m_Pos = pCollImpact->ImpactPoint.v3;
	Sphere.m_fRadius = 5.0f;
	UserTracker.MoveTracker( Sphere );

	// Find all trackers within range...
	m_pCallback_Tether = this;
	m_pCallback_ClosestBot = NULL;
	m_fCallback_ClosestDist2 = 0.0f;
	m_Callback_CollPoint.Set( pCollImpact->ImpactPoint );

	UserTracker.FindIntersectingTrackers( _FindTrackersInRangeCallback, FWORLD_TRACKERTYPE_MESH );

	if( m_pCallback_ClosestBot ) {
		// We hit a data port...

		bHitDataPort = TRUE;

		m_pTargetBot = m_pCallback_ClosestBot;
		m_pTargetBot->ImmobilizeBot();

		// Compute data port matrix...
		CFMtx43A Mtx;
		Mtx.UnitMtxFromNonUnitVec( m_pTargetBot->DataPort_GetNonUnitNormal() );
		Mtx.m_vPos = *m_pTargetBot->DataPort_GetPos();

		if( m_pUserProps->hParticleDataPortEffect ) {
			CEParticle *pEParticle = eparticlepool_GetEmitter();

			if( pEParticle ) {
				pEParticle->StartEmission( m_pUserProps->hParticleDataPortEffect, 1.0f, 5.0f, TRUE );

				pEParticle->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &Mtx, 1.0f, FALSE );
				m_pTargetBot->DataPort_AttachEntity_WS( pEParticle );
			}
		}

		CFSoundGroup::PlaySound( m_pUserProps->pSoundGroupDataPort, FALSE, &pCollImpact->ImpactPoint, -1, TRUE );

		// NKM
		if( m_pWeaponTether->GetOwner() ) {
			m_pSourceBot = m_pWeaponTether->GetOwner();
			m_pWeaponTether->GetOwner()->ImmobilizeBot();
		}
	} else {
		// We hit something else...
		m_pTargetBot = NULL;
	}

	// Cleanup...
	UserTracker.RemoveFromWorld();

	return bHitDataPort;
}


BOOL CTether::_FindTrackersInRangeCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	if( !pTracker->IsCollisionFlagSet() ) {
		// No collision possible with this tracker...
		return TRUE;
	}

	if( pTracker->m_nUser != MESHTYPES_ENTITY ) {
		// Not an entity...
		return TRUE;
	}

	// Tracker is a world mesh...
	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;

	if( pWorldMesh->m_nFlags & FMESHINST_FLAG_DONT_DRAW ) {
		// Mesh is not drawn, so skip...
		return TRUE;
	}

	// Tracker is a CEntity...
	CEntity *pEntity = (CEntity *)pWorldMesh->m_pUser;

	if( !(pEntity->TypeBits() & ENTITY_BIT_BOT) ) {
		// Not a bot...
		return TRUE;
	}

	// Entity is a bot...
	CBot *pBot = (CBot *)pEntity;

	if( !pBot->DataPort_IsOpen() ) {
		// Data port is not open (or is not installed)...
		return TRUE;
	}

	if( pBot->DataPort_IsReserved() ) {
		// Someone else just hooked into this dataport!
		return TRUE;
	}

	u32 i;

	// Skip if in our skip list...
	for( i=0; i<FWorld_nTrackerSkipListCount; ++i ) {
		if( pTracker == FWorld_apTrackerSkipList[i] ) {
			return TRUE;
		}
	}

	// This is a bot with an open data port...
	CFVec3A ImpactPos_WS;

	// Check distance from data port...
	f32 fDistToDataPort2 = pBot->DataPort_GetPos()->DistSq( m_Callback_CollPoint );
	if( fDistToDataPort2 > (_TETHER_DATA_PORT_RANGE*_TETHER_DATA_PORT_RANGE) ) {
		// Too far...
		return TRUE;
	}

	// Check angle to data port...
	CFVec3A DataPortUnitNormal_WS;
	pBot->DataPort_ComputeUnitNormal( &DataPortUnitNormal_WS );
	f32 fDot = DataPortUnitNormal_WS.Dot( m_pCallback_Tether->m_ModelMtx.m_vFront );
	if( fDot > -0.05f ) {
		// Too much of an angle...
		return TRUE;
	}

	// We hit a data port...

	if( (m_pCallback_ClosestBot == NULL) || (fDistToDataPort2 < m_fCallback_ClosestDist2) ) {
		// This data port is the closest so far...

		m_pCallback_ClosestBot = pBot;
		m_fCallback_ClosestDist2 = fDistToDataPort2;
	}

	return TRUE;
}


void CTether::_BuildTrackerSkipList( void ) {
	FWorld_nTrackerSkipListCount = 0;

	if( m_pWeaponTether ) {
		if( m_pWeaponTether->GetOwner() ) {
			m_pWeaponTether->GetOwner()->AppendTrackerSkipList();
		} else {
			m_pWeaponTether->AppendTrackerSkipList();
		}
	}
}


void CTether::_Work_PossessMode_FlyAlongCable( void ) {
	FASSERT( m_pWeaponTether != NULL );
	FASSERT( m_pWeaponTether->GetOwner()->m_nPossessionPlayerIndex >= 0 );

	s32 nPlayerIndex = m_pWeaponTether->GetOwner()->m_nPossessionPlayerIndex;
	CPlayer *pPlayer = &Player_aPlayer[ nPlayerIndex ];

	if( m_pTargetBot->IsDeadOrDying() ) {
		// kill the tether because the target bot is died
		Kill();
		return;
	}

	// Update camera animation along tether cable...
	m_fUnitCamAnim += FLoop_fPreviousLoopSecs * 0.3f * m_fPossessSpeed;
	if( m_fUnitCamAnim >= 1.0f ) {
		f32 fPossessionArmorModifier;

		fPossessionArmorModifier = m_pTargetBot->GetArmorModifier() + m_pUserProps->fPossessionArmorModifierDelta * (1.0f - m_pTargetBot->GetArmorModifier());
		FMATH_CLAMP( fPossessionArmorModifier, -1.0f, 1.0f );

		CFSoundGroup::PlaySound( m_pUserProps->pSoundGroupDeRes );

		// We're done with this possess mode, so go to next mode...
		m_nPossessMode = POSSESSMODE_FADE_IN_BEHIND_BOT;

		fcamera_GetCameraByIndex(nPlayerIndex)->SetFOV( m_fStartHalfFOV );
		// NKM - Once we possess, no more stun movement
		fcamera_GetCameraByIndex(nPlayerIndex)->StunCamera( -1.0f );

		// Transfer over to possessed Mil...
		FASSERT( m_pWeaponTether->GetOwner()->m_nPossessionPlayerIndex >= 0 );
		m_pTargetBot->Possess( m_pWeaponTether->GetOwner()->m_nPossessionPlayerIndex, fPossessionArmorModifier );

		// Remove Mil logo...
		pPlayer->SetDrawOverlayFunction( NULL );
		FMATH_CLEARBITMASK( m_nFlags, FLAG_DRAWING_MIL_LOGO | FLAG_CONTACT );

		// Start reducing view white saturation...
		CFColorRGBA StartColorRGBA, EndColorRGBA;
		StartColorRGBA.OpaqueWhite();
		EndColorRGBA.TransparentWhite();

		pPlayer->StartViewFade( &StartColorRGBA, &EndColorRGBA, 0.5f );

		return;
	}

	// Handle Mil logo fade-in...
	if( !(m_nFlags & FLAG_DRAWING_MIL_LOGO) ) {
		if( m_fUnitCamAnim > 0.25f ) {
			// Start Mil logo fade-in...

			FMATH_SETBITMASK( m_nFlags, FLAG_DRAWING_MIL_LOGO );
			m_fUnitDrawMilLogo = 0.0f;
			pPlayer->SetDrawOverlayFunction( _DrawMilLogoOverlay, this );
		}
	} else {
		if( m_fUnitDrawMilLogo < 1.0f ) {
			// Animate Mil logo fade-in...

			m_fUnitDrawMilLogo += FLoop_fPreviousLoopSecs * 0.75f * m_fPossessSpeed;
			FMATH_CLAMPMAX( m_fUnitDrawMilLogo, 1.0f );
		}
	}

	// Handle view white-saturation...
	if( !(m_nFlags & FLAG_WHITESAT_VIEW) ) {
		if( m_fUnitCamAnim > 0.35f ) {
			// Start the white-saturation...

			FMATH_SETBITMASK( m_nFlags, FLAG_WHITESAT_VIEW );

			CFColorRGBA StartColorRGBA, EndColorRGBA;
			StartColorRGBA.TransparentWhite();
			EndColorRGBA.OpaqueWhite();

			pPlayer->StartViewFade( &StartColorRGBA, &EndColorRGBA, fmath_Inv( m_fPossessSpeed ) );
		}
	}

	// Compute new camera orientation...
	CFMtx43A SourceCamMtx;
	CFVec3A TargetPos;
	CFQuatA SourceQuat, TargetQuat;
	f32 fUnitInterp, fHalfFOV;

	fUnitInterp = fmath_UnitLinearToSCurve( 1.0f - m_fUnitCamAnim );
	fUnitInterp *= fUnitInterp * fUnitInterp;
	fUnitInterp = 1.0f - fUnitInterp;

	SourceCamMtx.Mul( *m_pWeaponTether->GetOwner()->MtxToWorld(), m_DeltaCamMtx );
	SourceQuat.BuildQuat( SourceCamMtx );
	TargetQuat.BuildQuat( m_ModelMtx );

	TargetPos.Mul( m_ModelMtx.m_vFront, -0.5f ).Add( m_ModelMtx.m_vPos );
	TargetPos.y += 0.75f;
	m_CamInfo.m_Quat_WS.ReceiveSlerpOf( fUnitInterp, SourceQuat, TargetQuat );
	m_CamInfo.m_Pos_WS.Lerp( fUnitInterp, SourceCamMtx.m_vPos, TargetPos );

	// Compute new camera FOV...
	if( m_fUnitDrawMilLogo > 0.0f ) {
		fUnitInterp = fmath_UnitLinearToSCurve( m_fUnitDrawMilLogo );
		fUnitInterp *= fUnitInterp;
		fHalfFOV = FMATH_FPOT( fUnitInterp, m_fStartHalfFOV, FMATH_DEG2RAD( 70.0f ) );

		fcamera_GetCameraByIndex(nPlayerIndex)->SetFOV( fHalfFOV );
	}
}


void CTether::_Work_PossessMode_FadeInBehindBot( void ) {
	FASSERT( m_pTargetBot->m_nPossessionPlayerIndex >= 0 );
	CPlayer *pPlayer = &Player_aPlayer[ m_pTargetBot->m_nPossessionPlayerIndex ];

	// Wait for white-sat to be completely removed...
	if( pPlayer->GetViewFadeUnitProgress() == 1.0f ) {
		// White-sat is gone...
		m_nPossessMode = POSSESSMODE_POWERING_UP_BOT;

		FMATH_CLEARBITMASK( m_nFlags, FLAG_WHITESAT_VIEW );
		pPlayer->StopViewFade();

		// Power-up bot...
		m_pTargetBot->Power( TRUE, 0.0f, m_fPossessSpeed );

		fcamera_GetCameraByIndex(m_pTargetBot->m_nPossessionPlayerIndex)->SetFOV( m_fStartHalfFOV );

		return;
	}

	f32 fHalfFOV;

	fHalfFOV = fmath_UnitLinearToSCurve( pPlayer->GetViewFadeUnitProgress() );
	fHalfFOV = FMATH_FPOT( fHalfFOV, FMATH_DEG2RAD( 50.0f ), m_fStartHalfFOV );

	fcamera_GetCameraByIndex(m_pTargetBot->m_nPossessionPlayerIndex)->SetFOV( fHalfFOV );
}


void CTether::_Work_PossessMode_PoweringUpBot( void ) {
	// See if we're done powering-up bot...
	if( m_pTargetBot->Power_IsPoweredUp() ) {
		// We're done powering up...

		// Enable pause screen and weapon select...
		CPauseScreen::SetEnabled( TRUE );

		// TODO: We need to make sure the right HUD is enabled for this player
		CHud2::GetCurrentHud()->SetWSEnable( TRUE );

		// Enable control...
		FASSERT( m_pTargetBot->m_nPossessionPlayerIndex >= 0 );
		CPlayer *pPlayer = &Player_aPlayer[ m_pTargetBot->m_nPossessionPlayerIndex ];
		pPlayer->EnableEntityControl();
		m_pTargetBot->MobilizeBot();

		// Unreserve the dataport so someone else can possess the bot
		m_pTargetBot->DataPort_Reserve( FALSE );

		m_nPossessMode = POSSESSMODE_NORMAL;

		// NKM
		CBot *pBot = m_pWeaponTether->GetOwner();

		if( pBot->TypeBits() & (ENTITY_BIT_BOTGLITCH | ENTITY_BIT_KRUNK) ) {
			pBot->BreakIntoPieces( -1.0f );
		} else {
			m_pWeaponTether->GetOwner()->ImmobilizeBot();
		}

		// Destroy the cable...
		FMATH_CLEARBITMASK( m_nFlags, FLAG_CONTACT );
		Kill();
	}
}


void CTether::_EmergencyDisconnect( void ) {
	// Enable pause screen and weapon select...
	CPauseScreen::SetEnabled( TRUE );

	if( m_pTargetBot ) {
		m_pTargetBot->DataPort_Shock( FALSE );
		m_pTargetBot->ForceQuickDataPortUnPlug();
		m_pTargetBot->DataPort_Reserve( FALSE );
	}

	if( m_pSourceBot ) {
		m_pSourceBot->MobilizeBot();

		s32 nPlayerIndex = m_pSourceBot->m_nPossessionPlayerIndex;
		CHud2::GetHudForPlayer(nPlayerIndex)->SetWSEnable( TRUE );
		FASSERT( nPlayerIndex >= 0 );
		gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM(nPlayerIndex), m_pSourceBot );

		// Remove Mil logo...
		CPlayer *pPlayer = &Player_aPlayer[ nPlayerIndex ];

		if( m_nFlags & FLAG_DRAWING_MIL_LOGO ) {
			pPlayer->SetDrawOverlayFunction( NULL );
			FMATH_CLEARBITMASK( m_nFlags, FLAG_DRAWING_MIL_LOGO );
		}

		// Start reducing view white saturation...
		CFColorRGBA EndColorRGBA;
		EndColorRGBA.TransparentWhite();

		pPlayer->StartViewFade( &EndColorRGBA, &EndColorRGBA, 0.0f );

	}

	m_pTargetBot = NULL;
	m_pSourceBot = NULL;	
}



