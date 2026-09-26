//////////////////////////////////////////////////////////////////////////////////////
// Splat.cpp - Splat SFX class, for flat textures mapped to the ground
//
// Author: Mike Elliott
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
// 11.18.02 Elliott		Created.
//////////////////////////////////////////////////////////////////////////////////////

#if 0

#include "splat.h"
#include "fresload.h"
#include "fdraw.h"
#include "frenderer.h"
#include "fcoll.h"
#include "fworld_coll.h"
#include "floop.h"
#include "fdraw.h"
#include "player.h"


#define _MAX_SPLATS				4

#define _SPLAT_VTX_PER_SIDE		5
#define _SPLAT_NUM_STRIPS		(_SPLAT_VTX_PER_SIDE-1)
#define _SPLAT_VTX_PER_STRIP	(_SPLAT_VTX_PER_SIDE*2)
#define _SPLAT_TOTAL_VTX		(_SPLAT_NUM_STRIPS*_SPLAT_VTX_PER_STRIP)
#define _XZDELTA				(1.0f/(f32)(_SPLAT_VTX_PER_SIDE-1))
#define _HALF_COLLISION_RAY		4.0f
#define _VERT_DISPLACEMENT		0.25f			//displacement off surface



BOOL		CSplat::m_bSystemInitialized	= FALSE;				// TRUE if the system has been initialized
BOOL		CSplat::m_bSystemActive			= FALSE;
CSplat	   *CSplat::m_aSplats;
CFTexInst	CSplat::m_aTextures[SPLATTYPE_COUNT];

cchar	   *CSplat::m_apszSquishedTexNames[] = {
	"TRDMsqushY1",		// SPLATTYPE_SQUISHED_GLITCH,	
	"TRMGsqushR1"		// SPLATTYPE_SQUISHED_GRUNT,
};




BOOL CSplat::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );
	
	FResFrame_t Frame;
	
	Frame = fres_GetFrame();

	m_aSplats = fnew CSplat[_MAX_SPLATS];
	if( m_aSplats == NULL ) {
		DEVPRINTF( "CSplat::InitSystem(): Could not allocate CSplat array\n" );
		goto _ExitInitSystemWithError;
	}

	//initialize the textures
	if( !_InitTextures() ) {
		DEVPRINTF( "CSplat::InitSystem(): Could not initialize textures\n" );
		goto _ExitInitSystemWithError;
	}

	//init all the vertex arrays
	for( u32 i=0; i<_MAX_SPLATS; i++ ) {
		m_aSplats[i].m_aVtx = (FDrawVtx_t*)fres_Alloc( _SPLAT_TOTAL_VTX * sizeof( FDrawVtx_t) );
		if( m_aSplats[i].m_aVtx == NULL ) {
			DEVPRINTF( "CSplat::InitSystem(): Could not allocate vertex arrays\n" );
			goto _ExitInitSystemWithError;
		}

		// set the color now cause it won't change
		for( u32 j=0; j<_SPLAT_TOTAL_VTX; j++ ) {
			m_aSplats[i].m_aVtx[j].ColorRGBA.OpaqueWhite();
		}
	}

	m_bSystemInitialized = TRUE;
	return TRUE;

	// Error:
_ExitInitSystemWithError:
	UninitSystem();
	fres_ReleaseFrame( Frame );
	return FALSE;
}



void CSplat::UninitSystem( void ) {
	if( !m_bSystemInitialized ) {
		return;
	}

	if( m_aSplats ) {
		fdelete_array( m_aSplats );
	}

	m_bSystemInitialized = FALSE;
}


void CSplat::KillAll( void ) {
	for( u32 i=0; i<_MAX_SPLATS; i++ ) {
		m_aSplats[i].m_bActive = FALSE;
	}
	m_bSystemActive = FALSE;
}


void CSplat::AddSplat( u32 uType, f32 fSize, const CFVec3A &rvPos, const CFVec3A &rvFront ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( uType < SPLATTYPE_COUNT );

	CSplat *pSplat = _GetNewSplat();
	
	pSplat->m_Type	= (SplatType_e)uType;
	pSplat->m_fSize = fSize;

	// set up collision data
	CFCollInfo		collInfo;
	FCollImpact_t  *pImpact;

	collInfo.bCalculateImpactData		= TRUE;
	collInfo.bCullBacksideCollisions	= TRUE;
	collInfo.bFindClosestImpactOnly		= TRUE;
	collInfo.nStopOnFirstOfCollMask		= FCOLL_MASK_NONE;;
	collInfo.nCollMask					= FCOLL_MASK_WALKABLE;
	collInfo.nCollTestType				= FMESH_COLLTESTTYPE_RAY;
	collInfo.nResultsLOD				= FCOLL_LOD_HIGHEST;

	f32 fYHigh, fYLow;			//high and low spots to check for collisions
	f32 fRotY = FMATH_PI + fmath_Atan( rvFront.x, rvFront.z );
    
	fcoll_Clear();
	collInfo.Ray.Init( rvPos.x, rvPos.y + _HALF_COLLISION_RAY, rvPos.z, 
					   rvPos.x, rvPos.y - _HALF_COLLISION_RAY, rvPos.z );
	
	if( !fworld_CollideWithWorldTris( &collInfo ) ) {
		return;  // can't find the ground there
	} else {
		pImpact = &FColl_aImpactBuf[0];
		fYHigh = pImpact->ImpactPoint.y + _HALF_COLLISION_RAY;
		fYLow  = pImpact->ImpactPoint.y - _HALF_COLLISION_RAY;
	}

	f32 fx		= -(fSize*0.5f);
	f32 fxbase	= -(fSize*0.5f);
	f32 fz		= -(fSize*0.5f);
	f32 fdxz	= fSize * _XZDELTA;
	u32 uVtx;

	CFVec3A vPoint;

	for( u32 i=0; i<_SPLAT_VTX_PER_SIDE; i++ ) {			//Z
		for( u32 j=0; j<_SPLAT_VTX_PER_SIDE; j++ ) {		//X
			vPoint.Set( fx, 0.0f, fz );
			vPoint.RotateY( fRotY );
			vPoint.Add( rvPos );

			//do the collision thing
			fcoll_Clear();
			collInfo.Ray.Init(  vPoint.x, fYHigh, vPoint.z,
								vPoint.x, fYLow,  vPoint.z );
			
			if( !fworld_CollideWithWorldTris( &collInfo ) ) {
				return; // too steep, goodbye
			} else {
				pImpact = &FColl_aImpactBuf[0];
				vPoint.y = pImpact->ImpactPoint.y + _VERT_DISPLACEMENT;
			}

			// now assign this point to the proper vtxs
			if( i < _SPLAT_VTX_PER_SIDE-1) {
				uVtx = i*_SPLAT_VTX_PER_STRIP+(j*2)+1;
				pSplat->m_aVtx[uVtx].Pos_MS = vPoint.v3;
				pSplat->m_aVtx[uVtx].ST.Set( j*_XZDELTA, i*_XZDELTA );
			}
			if( i>0 ) {
				uVtx = (i-1)*_SPLAT_VTX_PER_STRIP+(j*2);
				pSplat->m_aVtx[uVtx].Pos_MS = vPoint.v3;
				pSplat->m_aVtx[uVtx].ST.Set( j*_XZDELTA, (i)*_XZDELTA );
			}

			fx += fdxz;
		}
		fx = fxbase;
		fz += fdxz;
	}

	pSplat->m_uTime		= FLoop_nTotalLoopTicks;
	pSplat->m_vPos		= rvPos.v3;
	pSplat->m_bActive	= TRUE;
	
	m_bSystemActive		= TRUE;
}


CSplat *CSplat::_GetNewSplat( void ) {
	u32 i;
	u64 uBestUnseenTime	 = FLoop_nTotalLoopTicks;
	u64 uBestSeenTime	 = FLoop_nTotalLoopTicks;
	u32	uBestUnseen		 = -1;
	u32 uBestSeen		 = -1;
	BOOL bFoundOffscreen = FALSE;

	for( i=0; i<_MAX_SPLATS; i++ ) {
		if( m_aSplats[i]._IsOffscreen() ) {
			if( m_aSplats[i].m_uTime < uBestUnseenTime ) {
				uBestUnseen = i;
				uBestUnseenTime = m_aSplats[i].m_uTime;
			}
		} else if( uBestUnseen == -1 ) { //if we already have an unseen one, don't bother
			if( m_aSplats[i].m_uTime < uBestSeenTime ) {
				uBestSeen = i;
				uBestSeenTime = m_aSplats[i].m_uTime;
			}
		}
	}

	if( uBestUnseen != -1 ) {
		return &m_aSplats[uBestUnseen];
	} else {
		if( uBestSeen == -1 ) {
			uBestSeen = 0;
		}
		return &m_aSplats[uBestSeen];
	}
}




BOOL CSplat::_InitTextures( void ) {
	FTexDef_t *pTexDef;
	
	for( u32 i=0; i<SPLATTYPE_COUNT; i++ ) {
		pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, m_apszSquishedTexNames[i] );
		if( pTexDef == NULL ) {
			DEVPRINTF( "CSplat::_InitTextures():  Error loading texture:  %s\n", m_apszSquishedTexNames[i] );
			return FALSE;
		}
		m_aTextures[i].SetTexDef( pTexDef );
	}

	return TRUE;
}


void CSplat::Work( void ) {
	//may want to consider slowly fading them out by playing w/ alpha
	if( m_bSystemActive ) {
		m_bSystemActive = FALSE;

		for( u32 i=0; i<_MAX_SPLATS; i++ ) {
			if( m_aSplats[i].m_bActive ) {
				m_bSystemActive = TRUE;
				break;
			}
		}
	}
}



// Assumes the FDraw renderer is currently active.
// Assumes the game's main perspective camera is
// set up. Assumes there are no model Xfms on the
// stack.
//
// Does not preserve the current FDraw state.
// Does (not) preserve the current viewport.
// Will preserve the Xfm stack.
// Will preserve the frenderer fog state.

void CSplat::Draw( void ) {
	if( !m_bSystemActive ) { 
		return;
	}

		FASSERT( m_bSystemInitialized );
	if( !m_bSystemActive ) {
		return;
	}

	frenderer_SetDefaultState();

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	//fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER );
	fdraw_Depth_SetBiasLevel( 1 );
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_SetCullDir( FDRAW_CULLDIR_CW );
	
	for( u32 i=0; i<_MAX_SPLATS; i++ ) {
		if( m_aSplats[i].m_bActive ) {
			m_aSplats[i]._Draw();
		}
	}
}



CSplat::CSplat() {
	m_bActive	= FALSE;
	m_aVtx		= NULL;
	m_fSize		= 1.0f;
	m_uTime		= 0;	
	m_vPos.Zero();
}




BOOL CSplat::_IsOffscreen( void ) {
	if( !m_bActive ) {
		return TRUE;
	}

	//check vs. viewing frustrum
	CFSphere sphTemp;
	sphTemp.m_Pos		= m_vPos;
	sphTemp.m_fRadius	= m_fSize * 0.5f;
	if( fviewport_TestSphere_WS( CPlayer::m_pCurrent->m_pViewportPersp3D, &sphTemp, FVIEWPORT_PLANESMASK_ALL ) != -1 ) {
		// in current frustrum
		return FALSE; // not offscreen
	}
    return TRUE;
}



void CSplat::_Draw( void ) {
	FASSERT( m_Type < SPLATTYPE_COUNT );

	//set the texture...
	fdraw_SetTexture( &m_aTextures[(u32)m_Type] );

	CFXfm xfm;
	xfm.Identity();
	xfm.PushModel();

	for( u32 i=0; i<_SPLAT_NUM_STRIPS; i++ ) {
		fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, &m_aVtx[i*_SPLAT_VTX_PER_STRIP], _SPLAT_VTX_PER_STRIP );
	}
	
    xfm.PopModel();
}

#endif