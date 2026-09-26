//////////////////////////////////////////////////////////////////////////////////////
// fxshockwave.cpp
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

#include "fxshockwave.h"
#include "floop.h"
#include "fcamera.h"
#include "frenderer.h"
#include "fresload.h"
#include "fworld_coll.h"
#include "bot.h"
#include "pspool.h"
#include "vehicle.h"
#include "meshtypes.h"

//POSSIBLY MORE TEXTURES LATER
#define _RING_TEX_FNAME				"TFF1dust001"

#define _RING_NUM_STRIPS			4
#define	_RING_NUM_FACETS			8
#define _RING_VTX_PER_STRIP			(2 * (4 * _RING_NUM_FACETS + 1))
#define _RING_NUM_VTX				(_RING_VTX_PER_STRIP * _RING_NUM_STRIPS * 2)
#define _RING_RING_WIDTH			2.0f
#define _RING_STRIP_DELTA			(_RING_RING_WIDTH / (f32)_RING_NUM_STRIPS)
#define _RING_TEXSCALE				0.45f
#define _RING_RADIUS				10.0f

#define _MAX_SHOCKWAVES				4

#define _PLAYER_PUSH_MODIFIER		0.5f		//amount to scale velocity impulse for a player (so they don't go out of world


//DUST CLOUD DEFINES
#define _MAX_DUST_PSPRITES			500
#define _DUST_TEXTURE				"TFP1Smoke01"
#define _DUST_CREATE_INTERVAL		0.005f
#define _DUST_INITIAL_ALPHA			0.15f

//THESE COULD ALL BE CONFIGURABLE
#define _DEFAULT_RING_WIDTH			2.0f
#define _DEFAULT_AMP_DECAY			2.5f
#define _DEFAULT_RAD_INCREASE		3.5f
#define	_DEFAULT_INITIAL_AMP		3.0f
#define _DEFAULT_INITIAL_SCALE		0.2f
#define _DEFAULT_CLEARANCE_HEIGHT	10.0f
#define _DEFAULT_PUSH_MAG			15.0f


//static member initialization
CFXShockwave   *CFXShockwave::m_pCollisionShockwave	= NULL;
CFXShockwave   *CFXShockwave::m_aFXShockwaves		= NULL;
BOOL			CFXShockwave::m_bSystemInitialized	= FALSE;
BOOL			CFXShockwave::m_bActive				= FALSE;
FDrawVtx_t	   *CFXShockwave::m_aRingVtx			= NULL;
f32			   *CFXShockwave::m_afRingVtxAlpha		= NULL;
CFTexInst		CFXShockwave::m_RingTexInst;
CFMtx43A		CFXShockwave::m_MtxRingTex;
CFPSpriteGroup	CFXShockwave::m_PSGroup;



struct _DustSprite_s {
	CFVec3	vPos;
	f32		fVel;		//only in y direction, accel is constant for all
	f32		fAlpha;		//should start at something & fade to zero based on velocity
	f32		fLifespan;
};


BOOL CFXShockwave::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );
	
	FResFrame_t Frame;
	
	Frame = fres_GetFrame();

	m_aFXShockwaves = fnew CFXShockwave[_MAX_SHOCKWAVES];
	if( m_aFXShockwaves == NULL ) {
		DEVPRINTF( "CFXShockwave::InitSystem(): Could not allocate shockwave array\n" );
		goto _ExitInitSystemWithError;
	}

	m_aRingVtx		 = (FDrawVtx_t*)fres_Alloc( sizeof( FDrawVtx_t ) * _RING_NUM_VTX );
	m_afRingVtxAlpha = (f32*)fres_Alloc( sizeof( f32 ) * _RING_NUM_VTX );

	if( (m_aRingVtx == NULL) || (m_afRingVtxAlpha == NULL) ) {
		DEVPRINTF( "CFXShockwave::InitSystem(): Could not allocate shockwave vertex data\n" );
		goto _ExitInitSystemWithError;
	}

	if( !_InitMesh() ) {
		DEVPRINTF( "CFXShockwave::InitSystem(): Could not initialize shockwave vertex data\n" );
		goto _ExitInitSystemWithError;
	}

	if( !_InitTex() ) {
		DEVPRINTF( "CFXShockwave::InitSystem(): Could not initialize texture data\n" );
		goto _ExitInitSystemWithError;
	}

	if( !_InitPointSprites() ) {
		DEVPRINTF( "CFXShockwave::InitSystem():  Could not initialize point sprites\n" );
		goto _ExitInitSystemWithError;
	}

	for( u32 i=0; i<_MAX_SHOCKWAVES; i++ ) {
		m_aFXShockwaves[i].m_fAmpY = 0.0f;
	}

	m_bSystemInitialized = TRUE;
	return TRUE;

	// Error:
_ExitInitSystemWithError:
	UninitSystem();
	fres_ReleaseFrame( Frame );
	return FALSE;
}



void CFXShockwave::UninitSystem( void ) {
	if( !m_bSystemInitialized ) {
		return;
	}

	if( m_aFXShockwaves ) {
		fdelete_array( m_aFXShockwaves );
	}


	m_bSystemInitialized = FALSE;
}


void CFXShockwave::KillAll( void ) {
	for( u32 i=0; i<_MAX_SHOCKWAVES; i++ ) {
		m_aFXShockwaves[_MAX_SHOCKWAVES].m_fAmpY = 0.0f;
	}
	m_bActive = FALSE;
}



BOOL CFXShockwave::_InitMesh( void ) {
	u32 nStrip, nFacet;
	f32 fInnerRadius1, fOuterRadius1, fInnerRadius2, fOuterRadius2, fInnerAngleY, fOuterAngleY, fInnerY, fOuterY, fAngleXZ, fX, fZ;
	f32 fInnerNormalY, fOuterNormalY, fInnerDot1, fOuterDot1, fInnerDot2, fOuterDot2, fAmbientLevel;
	f32 fInnerAlpha1, fOuterAlpha1, fInnerAlpha2, fOuterAlpha2, *pfVtxAlpha1, *pfVtxAlpha2;
	CFVec3 InnerUnitNormal1, OuterUnitNormal1, InnerUnitNormal2, OuterUnitNormal2, LightUnitDir;
	FDrawVtx_t *pVtx1, *pVtx2;

	LightUnitDir.Set( 1.0f, -1.0f, 1.0f );
	LightUnitDir.Unitize();

	fAmbientLevel = 0.4f;
	for( nStrip=0; nStrip<_RING_NUM_STRIPS; nStrip++ ) {
		pVtx1 = &m_aRingVtx[nStrip*2 * _RING_VTX_PER_STRIP];
		pVtx2 = &m_aRingVtx[(nStrip*2 + 1) * _RING_VTX_PER_STRIP];

		pfVtxAlpha1 = &m_afRingVtxAlpha[(nStrip*2) * _RING_VTX_PER_STRIP];
		pfVtxAlpha2 = &m_afRingVtxAlpha[(nStrip*2 + 1) * _RING_VTX_PER_STRIP];

		fInnerAngleY = FMATH_PI * (f32)nStrip * fmath_Inv((f32)_RING_NUM_STRIPS);
		fOuterAngleY = FMATH_PI * (f32)(nStrip+1) * fmath_Inv((f32)_RING_NUM_STRIPS);

		fInnerY = 0.5f + 0.5f*fmath_Cos( fInnerAngleY );
		fOuterY = 0.5f + 0.5f*fmath_Cos( fOuterAngleY );

		fInnerNormalY = 0.5f + 0.5f*(0.5f*fmath_Cos( fInnerAngleY * 2.0f ) + 0.5f);
		fOuterNormalY = 0.5f + 0.5f*(0.5f*fmath_Cos( fOuterAngleY * 2.0f ) + 0.5f);

		fInnerAlpha1 = 1.0f;
		fOuterAlpha1 = 1.0f;
		fInnerAlpha2 = 1.0f - (f32)nStrip	  * fmath_Inv((f32)_RING_NUM_STRIPS);
		fOuterAlpha2 = 1.0f - (f32)(nStrip+1) * fmath_Inv((f32)_RING_NUM_STRIPS);

		fInnerRadius1 = _RING_RADIUS + (f32)nStrip		* _RING_STRIP_DELTA;
		fOuterRadius1 = _RING_RADIUS + (f32)(nStrip+1) * _RING_STRIP_DELTA;
		fInnerRadius2 = _RING_RADIUS - (f32)nStrip		* _RING_STRIP_DELTA;
		fOuterRadius2 = _RING_RADIUS - (f32)(nStrip+1) * _RING_STRIP_DELTA;

		for( nFacet=0; nFacet<(_RING_NUM_FACETS*4 + 1); nFacet++ ) {
			fAngleXZ = FMATH_2PI * (f32)nFacet * fmath_Inv((f32)(_RING_NUM_FACETS*4));
			fmath_SinCos( fAngleXZ, &fX, &fZ );

			InnerUnitNormal1.x = fX;
			InnerUnitNormal1.y = fInnerNormalY;
			InnerUnitNormal1.z = fZ;
			OuterUnitNormal1.x = fX;
			OuterUnitNormal1.y = fOuterNormalY;
			OuterUnitNormal1.z = fZ;

			InnerUnitNormal2.x = -fX;
			InnerUnitNormal2.y = fInnerNormalY;
			InnerUnitNormal2.z = -fZ;
			OuterUnitNormal2.x = -fX;
			OuterUnitNormal2.y = fOuterNormalY;
			OuterUnitNormal2.z = -fZ;

			InnerUnitNormal1.Unitize();
			OuterUnitNormal1.Unitize();
			InnerUnitNormal2.Unitize();
			OuterUnitNormal2.Unitize();

			fInnerDot1 = InnerUnitNormal1.Dot( LightUnitDir );
			fOuterDot1 = OuterUnitNormal1.Dot( LightUnitDir );
			fInnerDot2 = InnerUnitNormal2.Dot( LightUnitDir );
			fOuterDot2 = OuterUnitNormal2.Dot( LightUnitDir );

			FMATH_CLAMPMIN( fInnerDot1, 0.0f );
			FMATH_CLAMPMIN( fOuterDot1, 0.0f );
			FMATH_CLAMPMIN( fInnerDot2, 0.0f );
			FMATH_CLAMPMIN( fOuterDot2, 0.0f );

			fInnerDot1 += fAmbientLevel;
			fOuterDot1 += fAmbientLevel;
			fInnerDot2 += fAmbientLevel;
			fOuterDot2 += fAmbientLevel;

			FMATH_CLAMPMAX( fInnerDot1, 1.0f );
			FMATH_CLAMPMAX( fOuterDot1, 1.0f );
			FMATH_CLAMPMAX( fInnerDot2, 1.0f );
			FMATH_CLAMPMAX( fOuterDot2, 1.0f );

			// Build Vtx 0 (outer):
			pVtx1->Pos_MS.x = fX * fOuterRadius1;
			pVtx1->Pos_MS.y = fOuterY;
			pVtx1->Pos_MS.z = fZ * fOuterRadius1;
			pVtx1->ColorRGBA.Set( fOuterDot1, fOuterAlpha1 );
			pVtx1->ST.Zero();
			//pVtx1->ColorRGBA.Set( 0.5f, fOuterAlpha1 );
			pVtx1++;
			

			// Build Vtx 1 (inner):
			pVtx1->Pos_MS.x = fX * fInnerRadius1;
			pVtx1->Pos_MS.y = fInnerY;
			pVtx1->Pos_MS.z = fZ * fInnerRadius1;
			pVtx1->ColorRGBA.Set( fInnerDot1, fInnerAlpha1 );
			pVtx1->ST.Zero();
			//pVtx1->ColorRGBA.Set( 0.5f, fOuterAlpha1 );
			pVtx1++;

			// Build Vtx 0 (inner):
			pVtx2->Pos_MS.x = fX * fInnerRadius2;
			pVtx2->Pos_MS.y = fInnerY;
			pVtx2->Pos_MS.z = fZ * fInnerRadius2;
			pVtx2->ColorRGBA.Set( fInnerDot2, fInnerAlpha2 );
			pVtx2->ST.Zero();
			//pVtx2->ColorRGBA.Set( 0.5f, fInnerAlpha2 );
			pVtx2++;

			// Build Vtx 1 (outer):
			pVtx2->Pos_MS.x = fX * fOuterRadius2;
			pVtx2->Pos_MS.y = fOuterY;
			pVtx2->Pos_MS.z = fZ * fOuterRadius2;
			pVtx2->ColorRGBA.Set( fOuterDot2, fOuterAlpha2 );
//			pVtx2->ColorRGBA.Set( 0.5f, fOuterAlpha2 );
			pVtx2->ST.Zero();
			pVtx2++;

			*pfVtxAlpha1++ = fOuterAlpha1;
			*pfVtxAlpha1++ = fInnerAlpha1;
			*pfVtxAlpha2++ = fInnerAlpha2;
			*pfVtxAlpha2++ = fOuterAlpha2;
		}
	}

	return TRUE;
}



BOOL CFXShockwave::_InitTex( void ) {
	//load the tex
	FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _RING_TEX_FNAME );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CFXShockwave::_InitTex(): Could not load ring dust texture.\n" );
		return FALSE;
	}
	m_RingTexInst.SetTexDef( pTexDef );
	m_RingTexInst.SetFlags( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T );

	m_MtxRingTex.Identity();
	m_MtxRingTex.aa[0][0] = _RING_TEXSCALE;
	m_MtxRingTex.aa[1][1] = 0.0f;
	m_MtxRingTex.aa[2][1] = _RING_TEXSCALE;
	m_MtxRingTex.aa[1][2] = _RING_TEXSCALE;
	m_MtxRingTex.aa[2][2] = 0.0f;

	return TRUE;
}

BOOL CFXShockwave::_InitPointSprites( void ) {
	// Build point sprite group...
	for( u32 i=0; i<_MAX_SHOCKWAVES; i++ ) {
		m_aFXShockwaves[i].m_aDustSpriteData = (_DustSprite_s*)fres_Alloc( _MAX_DUST_PSPRITES * sizeof( _DustSprite_s ) );
		if( m_aFXShockwaves[i].m_aDustSpriteData == NULL ) {
			return FALSE;
		}
	}

		// Load dust particle texture...
	FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _DUST_TEXTURE );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CFXShockwave::_InitPointSprites(): Could not find texture '%s'.\n", _DUST_TEXTURE );
	}



	m_PSGroup.m_Xfm.Identity();
	m_PSGroup.m_BoundSphere_MS.m_Pos.Zero();
	m_PSGroup.m_BoundSphere_MS.m_fRadius	= 15.0f;
	m_PSGroup.m_fCullDist					= 500.0f;
	m_PSGroup.m_nPSFlags					= FPSPRITE_FLAG_NONE;
	m_PSGroup.m_nBlend						= FPSPRITE_BLEND_MODULATE;
//	m_PSGroup.m_TexInst						= m_RingTexInst;
	m_PSGroup.m_TexInst.SetTexDef( pTexDef );
//	m_PSGroup.m_TexInst.SetFlags( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T );

	m_PSGroup.m_nMaxCount					= _MAX_DUST_PSPRITES;
	m_PSGroup.m_pBase						= NULL;
	m_PSGroup.m_nRenderCount				= 0;
	m_PSGroup.m_nRenderStartIndex			= 0;

	return TRUE;
}



void CFXShockwave::AddShockwave( const CFVec3A &vPos,												// center of the ring
								 f32 fInitialAmplitude,												// amplitude (Y direction)
								 f32 fSpeed,														// speed the ring expands
								 u64 uEntityFlags,													// what kind of entities to check collisions against
								 f32 fMaxPushEffect,												// how far we want to push entities 
								 f32 fMinPushEffect,												// min push
								 FWorldIntersectingTrackerCallback_t *pCollCallback/*=NULL*/ )	{	// callback for when we hit an entity

	FASSERT( m_bSystemInitialized );
	m_bActive = TRUE;
	CFXShockwave *pNewShockwave = _GetNewShockwave();
	if( pNewShockwave == NULL ) {
		return;
	}

	pNewShockwave->m_Sphere.m_Pos			= vPos.v3;
	pNewShockwave->m_fAmpY					= fInitialAmplitude;
	pNewShockwave->m_fMaxPushMag			= fMaxPushEffect;
	pNewShockwave->m_fMinPushMag			= fMinPushEffect;
	pNewShockwave->m_fScaleXZ				= pNewShockwave->m_fInitialScale;
	pNewShockwave->m_pCallbackFn			= pCollCallback;
	pNewShockwave->m_uEntityCollisionFlags	= uEntityFlags;

	pNewShockwave->m_bDrawingDust			= TRUE;
	pNewShockwave->m_fDustCreateTimer		= _DUST_CREATE_INTERVAL;
}



void CFXShockwave::Work( void ) {
	FASSERT( m_bSystemInitialized );

	if( !m_bActive ) {
		return;
	}

	m_bActive = FALSE;

	for( u32 i=0; i<_MAX_SHOCKWAVES; i++ ) {
		if( m_aFXShockwaves[i].m_fAmpY > 0.0f || m_aFXShockwaves[i].m_bDrawingDust ) {
			m_aFXShockwaves[i]._Work();
			m_bActive = TRUE;
		}
	}
}



CFXShockwave *CFXShockwave::_GetNewShockwave( void ) {
	FASSERT( m_bSystemInitialized );
	u32 i;
	f32 fLowestMag	= 100000.0f;
	u32 uBestSlot	= 0;

	for( i=0; i<_MAX_SHOCKWAVES; i++ ) {
		if( m_aFXShockwaves[i].m_fAmpY < fLowestMag ) {
			uBestSlot	= i;
			fLowestMag	= m_aFXShockwaves[i].m_fAmpY;
		}
	}
    
	FASSERT( uBestSlot < _MAX_SHOCKWAVES );

	return &m_aFXShockwaves[uBestSlot];
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

void CFXShockwave::Draw( void ) {
	FASSERT( m_bSystemInitialized );

	
	if( !m_bActive ) {		// work will clear this if they're all gone
		return;
	}

	frenderer_SetDefaultState();

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER );
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_SetCullDir( FDRAW_CULLDIR_CW );

	CFMtx43A::m_Temp.Mul( m_MtxRingTex, FXfm_pView->m_MtxR );
	FDrawTexCoordXfmMode_e oldTexMode;
	oldTexMode = fdraw_GetTexCoordXfmMode();
	fdraw_SetTexCoordXfmMode( FDRAW_TEXCOORDXFMMODE_CAMSPACE_POS );
	fdraw_SetTexCoordXfmMtx( &CFMtx43A::m_Temp );
	fdraw_SetTexture( &m_RingTexInst );
	
	for( u32 i=0; i<_MAX_SHOCKWAVES; i++ ) {
		if( m_aFXShockwaves[i].m_fAmpY > 0.0f ) {
			m_aFXShockwaves[i]._Draw();
		}
	}

	fdraw_SetTexCoordXfmMode( oldTexMode );

	
	frenderer_Push( FRENDERER_PSPRITE, NULL );
	frenderer_SetDefaultState();
	for( u32 i=0; i<_MAX_SHOCKWAVES; i++ ) {
		if( m_aFXShockwaves[i].m_bDrawingDust ) {
			m_aFXShockwaves[i]._DrawDust();
		}
	}
	frenderer_Pop();

}


#define _SHOCKWAVE_MASK_TIME			( 1.0f )
#define _VEHICLE_SHOCKWAVE_MULTIPLIER	( 25.0f )

BOOL CFXShockwave::_RingCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	FASSERT( m_pCollisionShockwave != NULL );
	FASSERT( pTracker->GetType() == FWORLD_TRACKERTYPE_MESH );
	FASSERT( pTracker->GetUserTypeBits() & ENTITY_BIT_BOT );

	if( m_pCollisionShockwave->m_pCallbackFn != NULL ) {
		return m_pCollisionShockwave->m_pCallbackFn( pTracker, pVolume );
	}

	CFVec3A vTmp;
	vTmp.Sub( m_pCollisionShockwave->m_Sphere.m_Pos, pTracker->GetBoundingSphere().m_Pos );

	CBot *pBot = (CBot*)pTracker->m_pUser;

	if( vTmp.MagSq() < ((m_pCollisionShockwave->m_Sphere.m_fRadius - m_pCollisionShockwave->m_fRingWidth) * (m_pCollisionShockwave->m_Sphere.m_fRadius - m_pCollisionShockwave->m_fRingWidth)) ) {
		return TRUE;	// too close
	}

	//now do the push away thing
	CFVec3A vecPush;
	f32 fIntensity = pTracker->GetBoundingSphere().m_Pos.y - m_pCollisionShockwave->m_Sphere.m_Pos.y;
	FMATH_FABS( fIntensity );
	fIntensity = (m_pCollisionShockwave->m_fClearanceHeight - fIntensity) * (m_pCollisionShockwave->m_fOOClearanceHeight);
	if( fIntensity < 0.0f ) {	// successfully jumped over
		return TRUE;
	}
	fIntensity *= fmath_Div( m_pCollisionShockwave->m_fAmpY, m_pCollisionShockwave->m_fInitialAmp );
	FMATH_CLAMP_UNIT_FLOAT(fIntensity);
	
	vecPush.Sub( pTracker->GetBoundingSphere().m_Pos, m_pCollisionShockwave->m_Sphere.m_Pos );
	vecPush.y += 5.0f;		// add some extra up force
	FMATH_CLAMPMIN( vecPush.y, 1.0f );
	vecPush.Unitize(); 

	// mask out shockwave effect for a short while to reduce chance of "riding the wave"
	u64 uDeltaTicks;
	if( FLoop_nTotalLoopTicks > pBot->m_uShockwaveResetTicks ) {
		uDeltaTicks = FLoop_nTotalLoopTicks - pBot->m_uShockwaveResetTicks;
	} else {
		uDeltaTicks = pBot->m_uShockwaveResetTicks - FLoop_nTotalLoopTicks;
	}

	if( uDeltaTicks > (u64) (_SHOCKWAVE_MASK_TIME * FLoop_fTicksPerSec) ) {
		if( pBot->TypeBits() & (ENTITY_BIT_VEHICLERAT | ENTITY_BIT_VEHICLESENTINEL) ) {
			// custom shockwave effect for vehicles
			CVehicle *pVehicle = (CVehicle*)pBot;
			CFVec3A vMomentum, vArm;

			// shake camera
			if( pBot->m_nPossessionPlayerIndex >= 0 ) {
				fcamera_GetCameraByIndex( pBot->m_nPossessionPlayerIndex )->ShakeCamera( fIntensity*0.25f, 0.25f );
			}

			// cook up a momentum value
			vMomentum.Set( 0.0f, fIntensity, 0.0f );
			vMomentum.Mul( _VEHICLE_SHOCKWAVE_MULTIPLIER );
			vMomentum.Mul( pVehicle->m_PhysObj.GetMass() );
	
			if( pBot->m_pBotInfo_Gen ) {
				vMomentum.Mul( pBot->m_pBotInfo_Gen->fUnitShockwaveEffect );
			}

			// construct an arm at which to apply momentum, so vehicle will rotate when it lifts.
			vArm.Set( m_pCollisionShockwave->m_Sphere.m_Pos );
			vArm.Sub( pVehicle->MtxToWorld()->m_vPos );
			vArm.Unitize();
			vArm.Mul( 10.0f );

			// apply momentum to vehicle
			pVehicle->m_PhysObj.ApplyMomentum( &vMomentum, &vArm );


		} else {
			CFVec3A vBotVel;
			f32 fSpd = pBot->m_Velocity_WS.Dot( vecPush );
			vBotVel.Mul( vecPush, fSpd );
			vecPush.Mul( FMATH_FPOT( fIntensity, m_pCollisionShockwave->m_fMinPushMag, m_pCollisionShockwave->m_fMaxPushMag ) * _PLAYER_PUSH_MODIFIER * pBot->m_pBotInfo_Gen->fUnitShockwaveEffect );
			vecPush.Sub( vBotVel );
			if( pBot->m_pBotInfo_Gen ) {
				vecPush.Mul( pBot->m_pBotInfo_Gen->fUnitShockwaveEffect );
			}
			pBot->ApplyVelocityImpulse_WS( vecPush );
			
			//// if this is a player controlled bot, don't push as much
			//if( pBot->m_nPossessionPlayerIndex >= 0 ) {
			//	fcamera_GetCameraByIndex( pBot->m_nPossessionPlayerIndex )->ShakeCamera( fIntensity*0.25f, 0.25f );
			//	FMATH_CLAMP( vecPush.y, 0.1f, 0.25f );
			//	vecPush.Mul( FMATH_FPOT( fIntensity, m_pCollisionShockwave->m_fMinPushMag, m_pCollisionShockwave->m_fMaxPushMag ) * _PLAYER_PUSH_MODIFIER * pBot->m_pBotInfo_Gen->fUnitShockwaveEffect );	// lower the intensity for a player bot
			//	pBot->ApplyVelocityImpulse_WS( vecPush );		
			//} else {
			//	vecPush.Mul( FMATH_FPOT( fIntensity, m_pCollisionShockwave->m_fMinPushMag, m_pCollisionShockwave->m_fMaxPushMag ) * _PLAYER_PUSH_MODIFIER * pBot->m_pBotInfo_Gen->fUnitShockwaveEffect );
			//	if( pBot->m_pBotInfo_Gen ) {
			//		vecPush.Mul( pBot->m_pBotInfo_Gen->fUnitShockwaveEffect );
			//	}
			//	pBot->ApplyVelocityImpulse_WS( vecPush );
			//}
		}

		pBot->m_uShockwaveResetTicks = FLoop_nTotalLoopTicks;
	}


	//// if this is a player controlled bot, don't push as much
	//if( pBot->m_nPossessionPlayerIndex >= 0 ) {
	//	fcamera_GetCameraByIndex( pBot->m_nPossessionPlayerIndex )->ShakeCamera( fIntensity*0.25f, 0.25f );
	//	FMATH_CLAMP( vecPush.y, 0.1f, 0.25f );
	//	vecPush.Mul( m_pCollisionShockwave->m_fPushMag * fIntensity * 0.5f );	// lower the intensity for a player bot
	//	pBot->ApplyVelocityImpulse_WS( vecPush );		
	//} else {
	//	vecPush.Mul( m_pCollisionShockwave->m_fPushMag * fIntensity );
	//	pBot->ApplyVelocityImpulse_WS( vecPush );	
	//}

	return TRUE;
}



CFXShockwave::CFXShockwave() {
	m_fRingWidth			= _DEFAULT_RING_WIDTH;				
	m_fAmpDecayRate			= _DEFAULT_AMP_DECAY;
	m_fRadIncreaseRate		= _DEFAULT_RAD_INCREASE;
	m_fInitialAmp			= _DEFAULT_INITIAL_AMP;
	m_fOOInitialAmp			= fmath_Inv( m_fInitialAmp );
	m_fInitialScale			= _DEFAULT_INITIAL_SCALE;
	m_fClearanceHeight		= _DEFAULT_CLEARANCE_HEIGHT;
	m_fOOClearanceHeight	= fmath_Inv( m_fClearanceHeight );
	m_fMaxPushMag			= _DEFAULT_PUSH_MAG;
	m_fMinPushMag			= 0.0f;
	m_fAmpY					= 0.0f;
	m_fScaleXZ				= 0.0f;
	m_Sphere.m_Pos.Zero();
	m_Sphere.m_fRadius		= 0.0f;
	m_pCallbackFn			= NULL;
	m_nNumDustSprites		= 0;
	m_bDrawingDust			= FALSE;
	m_fDustCreateTimer		= _DUST_CREATE_INTERVAL;
}


void CFXShockwave::_Work( void ) {
	if( m_fAmpY > 0.0f ) {
		m_fAmpY				-= FLoop_fPreviousLoopSecs * m_fAmpDecayRate;
		m_fScaleXZ			+= FLoop_fPreviousLoopSecs * m_fRadIncreaseRate;
		m_Sphere.m_fRadius	 = _RING_RADIUS * m_fScaleXZ;

		FMATH_CLAMP_MIN0( m_fAmpY );
		m_fDustCreateTimer -= FLoop_fPreviousLoopSecs;

		if( (m_fAmpY > 0.0f) && (m_fDustCreateTimer < 0.0f) && (m_nNumDustSprites < _MAX_DUST_PSPRITES) ) {
			//create a new dust puff at the ring
			f32 fMag2;
			CFVec3A vTmp;
			vTmp.Set( fmath_RandomBipolarUnitFloat(), 0.0f, fmath_RandomBipolarUnitFloat() );
			fMag2 = vTmp.MagSq();
			if( fMag2 > 0.0001f ) {
				vTmp.Mul( fmath_InvSqrt( fMag2 ) );
			} else {
				vTmp = CFVec3A::m_UnitAxisX;
			}

			vTmp.Mul( m_Sphere.m_fRadius );
			vTmp.Add( m_Sphere.m_Pos );

			m_aDustSpriteData[m_nNumDustSprites].vPos		= vTmp.v3;
			m_aDustSpriteData[m_nNumDustSprites].vPos.y	   += 0.5f * m_fAmpY;
			m_aDustSpriteData[m_nNumDustSprites].fVel		= -0.5f;
			m_aDustSpriteData[m_nNumDustSprites].fAlpha		= _DUST_INITIAL_ALPHA;
			m_aDustSpriteData[m_nNumDustSprites].fLifespan	= 0.0f;
			m_nNumDustSprites++;
			m_fDustCreateTimer = fmath_Div( _DUST_CREATE_INTERVAL, m_fScaleXZ );
		}

		//check for collisions...
		m_pCollisionShockwave = this;
		fworld_FindTrackersIntersectingSphere( &m_Sphere, FWORLD_TRACKERTYPE_MESH, _RingCollisionCallback, 0, NULL, NULL, MESHTYPES_ENTITY, m_uEntityCollisionFlags );
	}

	// ring is gone, but might still be drawing dust
	m_bDrawingDust = FALSE;
//	f32 fAccel = -96.0f;
	for( u32 i=0; i<m_nNumDustSprites; i++ ) {
		if( m_aDustSpriteData[i].vPos.y < (m_Sphere.m_Pos.y - 2.0f) ) {
			m_nNumDustSprites--;
			m_aDustSpriteData[i] = m_aDustSpriteData[m_nNumDustSprites];
			i--;
		} else {
			m_bDrawingDust = TRUE;
			m_aDustSpriteData[i].vPos.y += m_aDustSpriteData[i].fVel * FLoop_fPreviousLoopSecs;
			m_aDustSpriteData[i].fLifespan += FLoop_fPreviousLoopSecs;
			FMATH_CLAMP_UNIT_FLOAT( m_aDustSpriteData[i].fLifespan );
			m_aDustSpriteData[i].fAlpha  = FMATH_FPOT( m_aDustSpriteData[i].fLifespan, 0.15f, 0.05f );
		}
	}
}


void CFXShockwave::_Draw( void ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( m_fAmpY > 0.0f );
	
	u32 i;
	f32 fOOScaleXZ_WS,
		fOOScaleY_WS;

	CFXfm xfm;
	xfm.Identity();

	xfm.m_MtxF.aa[0][0] = m_fScaleXZ;
	xfm.m_MtxF.aa[1][1] = m_fAmpY;
	xfm.m_MtxF.aa[2][2] = m_fScaleXZ;
	xfm.m_MtxF.m_vPos	= m_Sphere.m_Pos;

	fOOScaleXZ_WS	= fmath_Inv(m_fScaleXZ);
	fOOScaleY_WS	= fmath_Inv(m_fAmpY);

	xfm.m_MtxR.aa[0][0] = fOOScaleXZ_WS;
	xfm.m_MtxR.aa[1][1] = fOOScaleY_WS;
	xfm.m_MtxR.aa[2][2] = fOOScaleXZ_WS;
	xfm.m_MtxR.m_vPos.Set( m_Sphere.m_Pos.x * fOOScaleXZ_WS, m_Sphere.m_Pos.y * fOOScaleY_WS, m_Sphere.m_Pos.z * fOOScaleXZ_WS );
	
	xfm.PushModel();

	//update the vertex alpha
	f32			*pfVertAlpha = &m_afRingVtxAlpha[0];
	FDrawVtx_t	*pVtx		 = &m_aRingVtx[0];
	f32	fAlpha = m_fAmpY * m_fOOInitialAmp;
    
	for( i=0; i<_RING_NUM_VTX; i++ ) {
		pVtx[i].ColorRGBA.fAlpha = pfVertAlpha[i] * fAlpha;
	}

	for( i=0; i<_RING_NUM_STRIPS; i++ ) {
		fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, &m_aRingVtx[(i*2) * _RING_VTX_PER_STRIP],	_RING_VTX_PER_STRIP );
		fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, &m_aRingVtx[(i*2+1) * _RING_VTX_PER_STRIP], _RING_VTX_PER_STRIP );
	}

    xfm.PopModel();
}


void CFXShockwave::_DrawDust( void ) {
	FPSprite_t *pPSprites = pspool_GetArray( m_nNumDustSprites );

	if( !pPSprites ) {
		//someone else got here first
		return;
	}

	for( u32 i=0; i<m_nNumDustSprites; i++ ) {
		pPSprites[i].fDim_MS	 = 5.0f;
		pPSprites[i].Point_MS	 = m_aDustSpriteData[i].vPos;
		//pPSprites[i].ColorRGBA.OpaqueWhite();
		pPSprites[i].ColorRGBA.Set( 0.5f, m_aDustSpriteData[i].fAlpha );
	}

	m_PSGroup.m_BoundSphere_MS	= m_Sphere;
	m_PSGroup.m_pBase			= pPSprites;
	m_PSGroup.m_nRenderCount	= m_nNumDustSprites;
	m_PSGroup.Render( FALSE );
	pspool_ReturnArray( pPSprites );
}



