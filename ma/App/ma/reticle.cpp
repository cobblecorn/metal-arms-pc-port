//////////////////////////////////////////////////////////////////////////////////////
// reticle.cpp - Targeting redicle module.
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
// 03/29/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "reticle.h"
#include "fmath.h"
#include "fviewport.h"
#include "flinklist.h"
#include "frenderer.h"
#include "fdraw.h"
#include "fres.h"
#include "fresload.h"
#include "ftex.h"
#include "fxfm.h"
#include "floop.h"
#include "fclib.h"
#include "player.h"
#include "fvtxpool.h"
#include "fcamera.h"
#include "entity.h"
#include "fworld_coll.h"
#include "game.h"
#include "bot.h"
#include "fsound.h"
#include "weapon.h"
#include "MultiplayerMgr.h"

#define _ANIMATE_SPEED						12.0f

#define _SOUND_GROUP_NAME_TARGET_IDENTIFIED	"TethID"
#define _SOUND_GROUP_NAME_TARGET_LOCK		"TethLock"

#define _LINESCALE_INNER_NORM_RADIUS		0.04f
#define _LINESCALE_OUTER_NORM_RADIUS		0.25f
#define _LINESCALE_X2Y_SCALE				0.15f
#define _HALORING_START_RADIUS				0.25f
#define _HALORING_END_RADIUS				0.05f

#define _PENT2_CORNER_ANGLE					FMATH_DEG2RAD( 35.0f )
#define _PENT2_NORM_THICKNESS				0.015f
#define _PENT2_NORM_GAP						0.01f

#define _TARGET_IDENTIFIED_ZOOM_SECS		0.33333f
#define _LOCKED_SWEEP_OFFSET				0.3f

#define _INACTIVE_RED						(176.0f/255.0f)
#define _INACTIVE_GREEN						(224.0f/255.0f)
#define _INACTIVE_BLUE						(255.0f/255.0f)
#define _INACTIVE_ALPHA						0.7f

#define _ACTIVE_RED							(207.0f/255.0f)
#define _ACTIVE_GREEN						(0.0f/255.0f)
#define _ACTIVE_BLUE						(0.0f/255.0f)
#define _ACTIVE_ALPHA						1.0f

#define _SCOPE_INFO_FIELD_CHAR_COUNT		10

#define _SCOPE_BORDER_STEP_SECS				0.5f

#define _SCOPE_ZOOM_TBOX_WIDTH			0.1f
#define _SCOPE_ZOOM_TBOX_HEIGHT			0.025f
#define _SCOPE_ZOOM_BORDER_THICKNESS	0.0f
#define _SCOPE_ZOOM_FONT_CODE			'8'

#define _AMMO_TEXT_TBOX_WIDTH			0.4f
#define _AMMO_TEXT_TBOX_HEIGHT			0.020f
#define _AMMO_TEXT_BORDER_THICKNESS		0.0f
#define _AMMO_TEXT_FONT_CODE			'9'
#define _AMMO_TEXT_FLASH_ON_SECS		0.75f
#define _AMMO_TEXT_FLASH_OFF_SECS		1.5f
#define _AMMO_TEXT_RED					1.0f
#define _AMMO_TEXT_GREEN				1.0f
#define _AMMO_TEXT_BLUE					1.0f



BOOL CReticle::m_bSystemInitialized = FALSE;
BOOL CReticle::m_bDrawAllEnabled = FALSE;
FLinkRoot_t CReticle::m_LinkRoot;
FDrawVtx_t CReticle::m_aFDrawVtx[8];
f32 CReticle::m_fHaloRingStep;
f32 CReticle::m_fPentVtxNormOffset;
f32 CReticle::m_fPentSideSin;
f32 CReticle::m_fPentSideCos;

CFSoundGroup *CReticle::m_pSoundGroupTetherIdentified;
CFSoundGroup *CReticle::m_pSoundGroupTetherLockBeep;

static char _szScopeZoomString[] = "~S0.75X~S1.00##";

CReticle::TexInfo_t CReticle::m_aTexInfo[TEXTURE_COUNT] = {
	// TEXTURE_NONE:
	NULL,				// pszTexName
	0.1f,				// fScale
	0.1f,				// fNormScreenRadius

	// TEXTURE_STD_CROSSHAIRS:
	"TFH2crossD1",		// pszTexName
	0.1f,				// fScale
	0.1f,				// fNormScreenRadius

	// TEXTURE_BLASTER:
	"TFH2crossD1",		// pszTexName
	0.1f,				// fScale
	0.1f,				// fNormScreenRadius

	// TEXTURE_FLAMER:
	"TFH2crossD1",		// pszTexName
	0.025f,				// fScale
	0.025f,				// fNormScreenRadius

	// TEXTURE_DROID_SCOPE1:
	"TFH2scope01",		// pszTexName
	0.025f,				// fScale
	0.025f,				// fNormScreenRadius

	// TEXTURE_DROID_SCOPE2:
	"TFH2scope02",		// pszTexName
	0.025f,				// fScale
	0.025f,				// fNormScreenRadius

	// TEXTURE_STATIC:
	"TFH1static1",		// pszTexName
	0.025f,				// fScale
	0.025f,				// fNormScreenRadius
	
	// TEXTURE_FLOORSENTRY:
	"TFH_cross01",		// pszTexName
	0.1f,				// fScale
	0.175f,				// fNormScreenRadius

	// TEXTURE_QUADLASER:
	"TFMPreticl1",		// pszTexName
	0.25f,				// fScale
	0.25f,				// fNormScreenRadius

	// TEXTURE_TANK_CANNON:
	"TFMCretic01",		// pszTexName
	0.078f,				// fScale
	0.25f,				// fNormScreenRadius
};


CFTexInst CReticle::m_aTexInst[TEXTURE_COUNT];


s32 CReticle::m_anTexIndex[TYPE_COUNT] = {
	-1,						//	TYPE_NONE
	TEXTURE_STD_CROSSHAIRS, //	TYPE_DROID_STANDARD
	TEXTURE_BLASTER,		//  TYPE_BLASTER
	-1,						//	TYPE_TETHER
	TEXTURE_FLAMER,			//	TYPE_FLAMER
	TEXTURE_STD_CROSSHAIRS, //	TYPE_DROID_SCOPE
	TEXTURE_FLOORSENTRY,	//	TYPE_FLOORSENTRY
	TEXTURE_QUADLASER,		//  TYPE_QUADLASER
	TEXTURE_TANK_CANNON,	//	TYPE_TANK_CANNON
};


f32 CReticle::m_afNormScreenRadius[TYPE_COUNT] = {
	0.1f,	// TYPE_NONE
	0.1f,	// TYPE_DROID_STANDARD
	0.1f,	// TYPE_BLASTER
	_LINESCALE_OUTER_NORM_RADIUS,	// TYPE_TETHER
	0.025f,	// TYPE_FLAMER
	0.025f,	// TYPE_DROID_SCOPE
	0.175f,	// TYPE_FLOORSENTRY
	0.15f,	// TYPE_QUADLASER
	0.05f,	// TYPE_TANK_CANNON
};




BOOL CReticle::InitSystem( void ) {
	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	FASSERT( !IsSystemInitialized() );

	flinklist_InitRoot( &m_LinkRoot, FANG_OFFSETOF( CReticle, m_Link ) );

	for( i=0; i<TEXTURE_COUNT; i++ ) {
		if( m_aTexInfo[i].pszTexName ) {
			m_aTexInst[i].SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, m_aTexInfo[i].pszTexName ) );

			if( m_aTexInst[i].GetTexDef() == NULL ) {
				DEVPRINTF( "CReticle::InitSystem(): Could not locate reticle texture '%s'.\n", m_aTexInfo[i].pszTexName );
				DEVPRINTF( "                        Reticles using this texture won't be drawn.\n" );
			}
		} else {
			m_aTexInst[i].SetTexDef( NULL );
		}
	}

	m_aTexInst[TEXTURE_DROID_SCOPE1].SetFlags( CFTexInst::FLAG_WRAP_T );
	m_aTexInst[TEXTURE_STATIC].SetFlags( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T );

	m_aFDrawVtx[0].ST.Set( 0.0f, 0.0f );
	m_aFDrawVtx[1].ST.Set( 0.0f, 1.0f );
	m_aFDrawVtx[2].ST.Set( 1.0f, 0.0f );
	m_aFDrawVtx[3].ST.Set( 1.0f, 1.0f );

	m_aFDrawVtx[4].ST.Set( 0.0f, 0.0f );
	m_aFDrawVtx[5].ST.Set( 0.0f, 1.0f );
	m_aFDrawVtx[6].ST.Set( 1.0f, 0.0f );
	m_aFDrawVtx[7].ST.Set( 1.0f, 1.0f );

	for( i=0; i<8; ++i ) {
		m_aFDrawVtx[i].Pos_MS.z = 100.0f;
	}

	m_fHaloRingStep = 1.0f / (f32)(HALO_RING_COUNT - 1);

	m_fPentVtxNormOffset = _PENT2_NORM_THICKNESS * fmath_Sin(_PENT2_CORNER_ANGLE) / fmath_Cos(_PENT2_CORNER_ANGLE);

	fmath_SinCos( FMATH_2PI / 5.0f, &m_fPentSideSin, &m_fPentSideCos );

	m_pSoundGroupTetherIdentified = CFSoundGroup::RegisterGroup( _SOUND_GROUP_NAME_TARGET_IDENTIFIED );
	m_pSoundGroupTetherLockBeep = CFSoundGroup::RegisterGroup( _SOUND_GROUP_NAME_TARGET_LOCK );

	m_bSystemInitialized = TRUE;

	return TRUE;
}


void CReticle::UninitSystem( void ) {
	if( IsSystemInitialized() ) {
		m_bSystemInitialized = FALSE;
	}
}


void CReticle::EnableDrawAll( BOOL bEnable ) {
	FASSERT( IsSystemInitialized() );
	m_bDrawAllEnabled = bEnable;
}


CReticle::CReticle() {
	m_nFlags = FLAG_NONE;
	m_bDrawAllEnabled = TRUE;
}


CReticle::~CReticle() {
	Destroy();
}


BOOL CReticle::Create( Type_e nType, u32 nPlayerIndex ) {
	u32 i;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( nType>=0 && nType<TYPE_COUNT );
	FASSERT( nPlayerIndex >= 0 );

	m_nFlags = FLAG_NONE;

	FMATH_SETBITMASK( m_nFlags, FLAG_CREATED );

	SetType( nType );
	m_nPlayerIndex = nPlayerIndex;

	SetNormScale( 1.0f );
	SetNormOrigin( 0.0f, 0.0f );

	m_InactiveColor.Set( _INACTIVE_RED, _INACTIVE_GREEN, _INACTIVE_BLUE, _INACTIVE_ALPHA );
	m_ActiveColor.Set( _ACTIVE_RED, _ACTIVE_GREEN, _ACTIVE_BLUE, _ACTIVE_ALPHA );

	m_nAmmoText = AMMO_TEXT_NONE;
	m_nAmmoTextFlashCount = 0;
	m_fAmmoTextFlashSecs = 0.0f;

	m_fUnitAnimScale = 0.0f;

	m_fScopeHeading = 0.0f;
	m_fScopePitch = 0.0f;
	m_fScopeZoomMultiplier = 1.0f;
	m_pScopeTargetedBot = NULL;
	m_fScopeInfoDisplaySecs = 0.0f;
	m_nScopeInfoDisplayCount = 0;
	m_nScopeInfoBorderIndex = 0;
	m_fScopeInfoBorderSecs = 0.0f;
	m_fScopeFlashMagSecs = 0.0f;
	m_nScopeFlashMagCount = 0;
	m_fScopeUnitJolt = 0.0f;

	for( i=0; i<RETICLE_SCOPE_INFO_SLOT_COUNT; ++i ) {
		m_afScopeInfoIntensity[i] = 0.0f;
	}

	TargetSys_Reset();
	EnableDraw( FALSE );

	AddToEnd( this );

	// Create scope zoom text box...
	FTextArea_t *pTextArea;

	pTextArea = ftext_GetAttributes( Player_aPlayer[ nPlayerIndex ].m_hTextBoxScopeZoom );

	pTextArea->bVisible = FALSE;
	pTextArea->fUpperLeftX = 0.5f - 0.5f*_SCOPE_ZOOM_TBOX_WIDTH;
	pTextArea->fLowerRightX = 0.5f + 0.5f*_SCOPE_ZOOM_TBOX_WIDTH;
	pTextArea->fUpperLeftY = 0.5f - 0.5f*_SCOPE_ZOOM_TBOX_HEIGHT;
	pTextArea->fLowerRightY = 0.5f + 0.5f*_SCOPE_ZOOM_TBOX_HEIGHT;
	pTextArea->fNumberOfLines = 1;
	pTextArea->fBorderThicknessX = _SCOPE_ZOOM_BORDER_THICKNESS;
	pTextArea->fBorderThicknessY = _SCOPE_ZOOM_BORDER_THICKNESS;
	pTextArea->oColorForeground.OpaqueWhite();
	pTextArea->oColorBackground.Set( 0.0f, 0.0f, 0.0f, 0.0f );
	pTextArea->oColorBorder.Set( 0.0f, 0.0f, 0.0f, 0.0f );
	pTextArea->ohFont = _SCOPE_ZOOM_FONT_CODE;
	pTextArea->oHorzAlign = FTEXT_HORZ_ALIGN_LEFT;

	// Create low/no ammo text box...
	pTextArea = ftext_GetAttributes( Player_aPlayer[ nPlayerIndex ].m_hTextBoxLowNoAmmo );

	pTextArea->bVisible = FALSE;
	pTextArea->fUpperLeftX = 0.5f;
	pTextArea->fLowerRightX = 0.5f + _AMMO_TEXT_TBOX_WIDTH;
	pTextArea->fUpperLeftY = 0.345f - 0.5f*_AMMO_TEXT_TBOX_HEIGHT;
	pTextArea->fLowerRightY = 0.345f + 0.5f*_AMMO_TEXT_TBOX_HEIGHT;
	pTextArea->fNumberOfLines = 1;
	pTextArea->fBorderThicknessX = _AMMO_TEXT_BORDER_THICKNESS;
	pTextArea->fBorderThicknessY = _AMMO_TEXT_BORDER_THICKNESS;
	pTextArea->oColorForeground.Set( _AMMO_TEXT_RED, _AMMO_TEXT_GREEN, _AMMO_TEXT_BLUE, 1.0f );
	pTextArea->oColorBackground.Set( 0.0f, 0.0f, 0.0f, 0.0f );
	pTextArea->oColorBorder.Set( 0.0f, 0.0f, 0.0f, 0.0f );
	pTextArea->ohFont = _AMMO_TEXT_FONT_CODE;
	pTextArea->oHorzAlign = FTEXT_HORZ_ALIGN_LEFT;
	pTextArea->bNoScale = TRUE;

	return TRUE;
}


void CReticle::Destroy( void ) {
	if( IsCreated() ) {
		FMATH_CLEARBITMASK( m_nFlags, FLAG_CREATED );
		Remove( this );
		m_bDrawAllEnabled = FALSE;
	}
}


void CReticle::TargetSys_UpdateTargetedPoint( const FViewport_t *pViewport, const CFVec3A *pTargetedPoint_WS ) {
	FASSERT( IsCreated() );

	m_TargetedPoint_SS.z = fviewport_ComputeUnitOrtho3DScreenPoint_WS( pViewport, &FXfm_pView->m_MtxF, pTargetedPoint_WS, &m_TargetedPoint_SS );
}


void CReticle::TargetSys_UpdateTargetedIntensity( f32 fUnitIntensity ) {
	FASSERT( IsCreated() );

	if( m_fTargetedUnitIntensity == fUnitIntensity ) {
		return;
	}

	if( fUnitIntensity == 0.0f ) {
		f32 fPrevLockOnSpeed = m_fTargetLockOnSpeed;
		TargetSys_Reset();
		m_fTargetLockOnSpeed = fPrevLockOnSpeed;
	} else {
		if( m_fTargetedUnitIntensity == 0.0f ) {
			// Start locking...

			m_fTargetedUnitZoom = 0.0f;

			if( !m_bTargetedIDBeepPlayed ) {
				CFSoundGroup::PlaySound( m_pSoundGroupTetherIdentified );
				m_bTargetedIDBeepPlayed = TRUE;
			}
		}

		m_fTargetedUnitIntensity = fUnitIntensity;
	}
}


void CReticle::TargetSys_Reset( void ) {
	u32 i;

	m_TargetedPoint_SS.Zero();
	m_fTargetLockOnSpeed = 0.5f;
	m_fTargetedUnitIntensity = 0.0f;
	m_fTargetedUnitZoom = 0.0f;
	m_fTargetedUnitLock = 0.0f;
	m_fTargetedUnitLockSweep = 0.0f;
	m_fTargetedLockRotAngle = 0.0f;
	m_bTargetedLockPlayBeep = FALSE;
	m_bTargetedIDBeepPlayed = FALSE;

	for( i=0; i<HALO_RING_COUNT; ++i ) {
		m_aHaloInfo[i].ColorRGB.White();
		m_aHaloInfo[i].fUnitIntensity = 0.0f;
		m_aHaloInfo[i].fRotAngle = 0.0f;
		m_aHaloInfo[i].fIntensityFadeSpeed = 0.0f;
		m_aHaloInfo[i].bFadeEnds = FALSE;
	}
}


void CReticle::TargetSys_SetLockOnSpeed( f32 fLockOnSpeed ) {
	FASSERT( IsCreated() );

	m_fTargetLockOnSpeed = fLockOnSpeed;
}


// Returns -1 if the point is not locked.
f32 CReticle::TargetSys_ComputeUnitInaccuracy( void ) const {
	if( m_fTargetedUnitIntensity<1.0f || m_fTargetedUnitZoom<1.0f ) {
		return -1.0f;
	}

	return 1.0f - m_fTargetedUnitLock;
}


BOOL CReticle::IsPointWithinReticle( const FViewport_t *pViewport, const CFVec3A *pPoint_WS ) const {
	CFVec3A Point_SS;
	f32 fPointDistFromReticleCenter2, fReticleRadius;

	fviewport_ComputeUnitOrtho3DScreenPoint_WS( pViewport, &FXfm_pView->m_MtxF, pPoint_WS, &Point_SS );

	Point_SS.x -= m_fNormOriginX;
	Point_SS.y -= m_fNormOriginY;

	fPointDistFromReticleCenter2 = Point_SS.x*Point_SS.x + Point_SS.y*Point_SS.y;
	fReticleRadius = m_afNormScreenRadius[m_nType];

	return (fPointDistFromReticleCenter2 <= fReticleRadius*fReticleRadius);
}


// Returns 0.0f if the point is in the dead center of the reticle.
// Returns 1.0f if the point is on the outside edge of the reticle.
// Returns >1.0f if the point is outside the reticle.
f32 CReticle::ComputeUnitWithinReticle( const FViewport_t *pViewport, const CFVec3A *pPoint_WS ) const {
	CFVec3A Point_SS;
	f32 fPointDistFromReticleCenter, fReticleRadius, fUnitInaccuracy;

	fviewport_ComputeUnitOrtho3DScreenPoint_WS( pViewport, &FXfm_pView->m_MtxF, pPoint_WS, &Point_SS );

	Point_SS.x -= m_fNormOriginX;
	Point_SS.y -= m_fNormOriginY;

	fPointDistFromReticleCenter = fmath_Sqrt( Point_SS.x*Point_SS.x + Point_SS.y*Point_SS.y );
	fReticleRadius = m_afNormScreenRadius[m_nType];
	fUnitInaccuracy = fmath_Div( fPointDistFromReticleCenter, fReticleRadius );

	return fUnitInaccuracy;
}


void CReticle::WorkAll( void ) {
	FTextArea_t *pTextArea;
	CReticle *pReticle;

	FASSERT( IsSystemInitialized() );

	for( pReticle=GetFirst(); pReticle; pReticle=GetNext( pReticle ) ) {
		FASSERT( pReticle->IsCreated() );

		if( pReticle->m_nType == TYPE_DROID_SCOPE ) {
			if( pReticle->m_fScopeFlashMagSecs > 0.0f ) {
				pReticle->m_fScopeFlashMagSecs -= FLoop_fPreviousLoopSecs;

				if( pReticle->m_fScopeFlashMagSecs <= 0.0f ) {
					if( pReticle->m_nScopeFlashMagCount > 0 ) {
						pReticle->m_fScopeFlashMagSecs = 0.1f;
						--pReticle->m_nScopeFlashMagCount;
					} else {
						pReticle->m_fScopeFlashMagSecs = 1.0f;
						pReticle->m_nScopeFlashMagCount = 6;
					}
				}
			}

			if( pReticle->m_fScopeUnitJolt > 0.0f ) {
				pReticle->m_fScopeUnitJolt -= 4.0f * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMIN( pReticle->m_fScopeUnitJolt, 0.0f );
			}

			if( pReticle->m_fScopeInfoBorderSecs > 0.0f ) {
				pReticle->m_fScopeInfoBorderSecs -= FLoop_fPreviousLoopSecs;

				if( pReticle->m_fScopeInfoBorderSecs <= 0.0f ) {
					FASSERT( pReticle->m_nScopeInfoBorderIndex < RETICLE_SCOPE_INFO_SLOT_COUNT );
					pReticle->m_afScopeInfoIntensity[ pReticle->m_nScopeInfoBorderIndex ] = 1.0f;

					++pReticle->m_nScopeInfoBorderIndex;
					pReticle->m_fScopeInfoBorderSecs = 0.035f;

					if( pReticle->m_nScopeInfoBorderIndex >= RETICLE_SCOPE_INFO_SLOT_COUNT ) {
						pReticle->m_nScopeInfoBorderIndex = 0;
						pReticle->m_fScopeInfoBorderSecs = 1.0f;
					}
				}
			}

			if( pReticle->m_nScopeInfoDisplayCount>=1 && pReticle->m_nScopeInfoDisplayCount<=RETICLE_SCOPE_INFO_SLOT_COUNT ) {
				pReticle->m_fScopeInfoDisplaySecs -= FLoop_fPreviousLoopSecs;

				if( pReticle->m_fScopeInfoDisplaySecs <= 0.0f ) {
					pReticle->m_fScopeInfoDisplaySecs = 0.035f;
					++pReticle->m_nScopeInfoDisplayCount;

					if( pReticle->m_nScopeInfoDisplayCount >= (RETICLE_SCOPE_INFO_SLOT_COUNT+1) ) {
						pTextArea = ftext_GetAttributes( Player_aPlayer[ pReticle->m_nPlayerIndex ].m_ahTextBoxScopeInfo[RETICLE_SCOPE_INFO_SLOT_COUNT - 1] );
						pTextArea->oColorBorder.fAlpha = 0.0f;

						pReticle->m_fScopeInfoBorderSecs = 0.035f;
						pReticle->m_nScopeInfoBorderIndex = 0;
					}
				}
			}

			if( pReticle->m_nScopeInfoDisplayCount >= 1 ) {
				u32 i;

				for( i=0; i<RETICLE_SCOPE_INFO_SLOT_COUNT; ++i ) {
					if( pReticle->m_afScopeInfoIntensity[i] > 0.0f ) {
						pReticle->m_afScopeInfoIntensity[i] -= 1.5f * FLoop_fPreviousLoopSecs;
						FMATH_CLAMPMIN( pReticle->m_afScopeInfoIntensity[i], 0.0f );
					}
				}
			}
		}
	}
}


// Assumes the FDraw renderer is currently active.
// Assumes the ortho3D camera is set up and positioned
// at (0,0,0) and looking down the +Z axis.
// Assumes there are no model Xfms on the stack.
//
// Does not preserve the current FDraw state.
// Does not preserve the current viewport.
// Will preserve the Xfm stack.
// Will preserve the frenderer fog state.
void CReticle::DrawAll( void ) {
	CReticle *pReticle;
	BOOL bDrewOne;

	FASSERT( IsSystemInitialized() );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( !m_bDrawAllEnabled ) {
		for( pReticle=GetFirst(); pReticle; pReticle=GetNext( pReticle ) ) {
			if( pReticle->m_nAmmoText != AMMO_TEXT_NONE ) {
				ftext_GetAttributes( Player_aPlayer[ pReticle->m_nPlayerIndex ].m_hTextBoxLowNoAmmo )->bVisible = FALSE;

				pReticle->m_nAmmoText = AMMO_TEXT_NONE;
				pReticle->m_nAmmoTextFlashCount = 0;
				pReticle->m_fAmmoTextFlashSecs = 0.0f;
			}
		}

		return;
	}

	bDrewOne = FALSE;

	for( pReticle=GetFirst(); pReticle; pReticle=GetNext( pReticle ) ) {
		FASSERT( pReticle->IsCreated() );

		if( pReticle->m_nType!=TYPE_NONE ) {
			if( pReticle->IsDrawEnabled() || // draw if draw's enable
			  ( (pReticle->m_fUnitAnimScale>0.0f)&&!pReticle->IsReticleOverriden() ) ) { // animate on/off if not overriden
				if( !bDrewOne ) {
					bDrewOne = TRUE;

					//bFogEnabled = frenderer_Fog_IsEnabled();

					frenderer_SetDefaultState();
					//frenderer_Fog_Enable( FALSE );

					fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
					fdraw_Depth_EnableWriting( FALSE );
					fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
				}

				if( pReticle->m_nAmmoText != AMMO_TEXT_NONE ) {
					pReticle->_DisplayAmmoText();
				}

				switch( pReticle->m_nType ) {
				case TYPE_DROID_STANDARD:
				case TYPE_FLAMER:
				case TYPE_FLOORSENTRY:
				case TYPE_TANK_CANNON:
				case TYPE_BLASTER:
					pReticle->_Draw_DroidStandard( pReticle->m_nType );
					break;

				case TYPE_QUADLASER:
					pReticle->_Draw_MirrorXY( pReticle->m_nType );
					break;

				case TYPE_TETHER:
					pReticle->_Draw_Tether();
					break;

				case TYPE_DROID_SCOPE:
					// For single-player, we don't draw here because for the
					// scope reticle, the drawing is done via a callback from
					// game.cpp so that we can use the fullscreen render target.
					// Multiplayer, however, draws a simplified scope reticle.
					if( CPlayer::m_nPlayerCount > 1 ) {
						pReticle->_Draw_Scope();
					}

					break;
				}
			}
		}

		if( pReticle->m_nAmmoText == _AMMO_TEXT_SET_TO_NONE ) {
			ftext_GetAttributes( Player_aPlayer[ pReticle->m_nPlayerIndex ].m_hTextBoxLowNoAmmo )->bVisible = FALSE;

			pReticle->m_nAmmoText = AMMO_TEXT_NONE;
			pReticle->m_nAmmoTextFlashCount = 0;
			pReticle->m_fAmmoTextFlashSecs = 0.0f;
		} else {
			pReticle->m_nAmmoText = _AMMO_TEXT_SET_TO_NONE;
		}
	}

	if( bDrewOne ) {
		//frenderer_Fog_Enable( bFogEnabled );
	}
}


// Draws TYPE_DROID_STANDARD.
void CReticle::_Draw_DroidStandard( Type_e nType ) {
	f32 fNormScale, fNormScaleY;
	s32 nTexIndex;

	nTexIndex = m_anTexIndex[nType];

	if( m_aTexInst[ nTexIndex ].GetTexDef() == NULL ) {
		return;
	}

	FViewport_t *pViewport = Player_aPlayer[m_nPlayerIndex].m_pViewportOrtho3D;
	fviewport_SetActive( pViewport );

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_SetTexture( &m_aTexInst[ nTexIndex ] );

	fNormScale = m_fNormScale * m_aTexInfo[ nTexIndex ].fScale;
	fNormScaleY = fNormScale * m_fUnitAnimScale;

	if( !(m_nFlags & FLAG_ACTIVE_COLOR) ) {
		// Inactive color...

		m_aFDrawVtx[0].ColorRGBA = m_InactiveColor;
		m_aFDrawVtx[1].ColorRGBA = m_InactiveColor;
		m_aFDrawVtx[2].ColorRGBA = m_InactiveColor;
		m_aFDrawVtx[3].ColorRGBA = m_InactiveColor;
	} else {
		// Active color...

		m_aFDrawVtx[0].ColorRGBA = m_ActiveColor;
		m_aFDrawVtx[1].ColorRGBA = m_ActiveColor;
		m_aFDrawVtx[2].ColorRGBA = m_ActiveColor;
		m_aFDrawVtx[3].ColorRGBA = m_ActiveColor;
	}

	m_aFDrawVtx[0].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX - fNormScale);
	m_aFDrawVtx[0].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY + fNormScaleY);

	m_aFDrawVtx[1].Pos_MS.x = m_aFDrawVtx[0].Pos_MS.x;
	m_aFDrawVtx[1].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY - fNormScaleY);

	m_aFDrawVtx[2].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX + fNormScale);
	m_aFDrawVtx[2].Pos_MS.y = m_aFDrawVtx[0].Pos_MS.y;

	m_aFDrawVtx[3].Pos_MS.x = m_aFDrawVtx[2].Pos_MS.x;
	m_aFDrawVtx[3].Pos_MS.y = m_aFDrawVtx[1].Pos_MS.y;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, m_aFDrawVtx, 4 );

	if( IsDrawEnabled() ) {
		if( m_fUnitAnimScale < 1.0f ) {
			m_fUnitAnimScale += _ANIMATE_SPEED * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fUnitAnimScale, 1.0f );
		}
	} else {
		if( m_fUnitAnimScale > 0.0f ) {
			m_fUnitAnimScale -= _ANIMATE_SPEED * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fUnitAnimScale, 0.0f );
		}
	}
}


void CReticle::_Draw_Tether( void ) {
	u32 i;

	FViewport_t *pViewport = Player_aPlayer[m_nPlayerIndex].m_pViewportOrtho3D;
	fviewport_SetActive( pViewport );

	if( m_fTargetedUnitLock==1.0f && m_fTargetedUnitZoom==1.0f && m_fTargetedUnitIntensity==1.0f ) {
		if( m_fTargetedUnitLockSweep > 0.5f ) {
			CFColorRGBA ColorRGBA;
			ColorRGBA.Set( 1.0f, 0.3f, 0.3f, 1.0f );
			_Draw_TetherLineScale( pViewport, &ColorRGBA, _LINESCALE_INNER_NORM_RADIUS, _LINESCALE_OUTER_NORM_RADIUS, 0.03f, m_fNormScale );
		} else {
			_Draw_TetherLineScale( pViewport, &FColor_MotifWhite, _LINESCALE_INNER_NORM_RADIUS, _LINESCALE_OUTER_NORM_RADIUS, 0.03f, m_fNormScale );
		}
	} else {
		_Draw_TetherLineScale( pViewport, &FColor_MotifWhite, _LINESCALE_INNER_NORM_RADIUS, _LINESCALE_OUTER_NORM_RADIUS, 0.03f, m_fNormScale );
	}

	// Handle targeting...
	if( (m_fTargetedUnitIntensity > 0.0f) && (m_TargetedPoint_SS.z > 0.0f) ) {
		// Targeting enabled...

		CFVec3A Origin_SS, OuterOrigin_SS;
		CFXfm Xfm, XlatXfm, ScaleXfm;
		f32 fUnitStep, fNormRadius, fNormEndRadius, fPrevVal;
		u32 nPrevHaloIndex, nNewHaloIndex;
		CFColorRGBA ColorRGBA;
		BOOL bDrawRing, bRotateRing;

		Origin_SS.z = 10.0f;
		fNormRadius = fNormEndRadius = 10.0f * fmath_Div( _HALORING_END_RADIUS, m_TargetedPoint_SS.z );

		bDrawRing = TRUE;
		bRotateRing = TRUE;
		if( m_fTargetedUnitLock==1.0f && m_fTargetedUnitZoom==1.0f && m_fTargetedUnitIntensity==1.0f ) {
			bDrawRing = (u32)(m_fTargetedUnitLockSweep * 4.0f) & 1;
			bRotateRing = FALSE;

			if( bDrawRing ) {
				if( m_bTargetedLockPlayBeep ) {
					CFSoundGroup::PlaySound( m_pSoundGroupTetherLockBeep );
					m_bTargetedLockPlayBeep = FALSE;
				}
			} else {
				m_bTargetedLockPlayBeep = TRUE;
			}
		}

		if( bDrawRing ) {
			// Update and draw inner ring and its halos...
			if( m_fTargetedUnitZoom == 1.0f ) {
				Origin_SS.x = m_TargetedPoint_SS.x;
				Origin_SS.y = m_TargetedPoint_SS.y;
			} else {
				Origin_SS.x = FMATH_FPOT( m_fTargetedUnitZoom, m_fNormOriginX, m_TargetedPoint_SS.x );
				Origin_SS.y = FMATH_FPOT( m_fTargetedUnitZoom, m_fNormOriginY, m_TargetedPoint_SS.y );
				fNormRadius = FMATH_FPOT( m_fTargetedUnitZoom, _HALORING_START_RADIUS, fNormEndRadius );

				nPrevHaloIndex = (u32)(m_fTargetedUnitZoom * ((f32)(HALO_RING_COUNT-1)));
				FASSERT( nPrevHaloIndex < HALO_RING_COUNT );

				m_fTargetedUnitZoom += FLoop_fPreviousLoopSecs * (1.0f / _TARGET_IDENTIFIED_ZOOM_SECS);
				FMATH_CLAMPMAX( m_fTargetedUnitZoom, 1.0f );

				nNewHaloIndex = (u32)(m_fTargetedUnitZoom * ((f32)(HALO_RING_COUNT-1)));
				FASSERT( nNewHaloIndex < HALO_RING_COUNT );

				if( nNewHaloIndex != nPrevHaloIndex ) {
					m_aHaloInfo[nNewHaloIndex].ColorRGB.White();
					m_aHaloInfo[nNewHaloIndex].fUnitIntensity = 0.5f;
					m_aHaloInfo[nNewHaloIndex].fRotAngle = m_fTargetedLockRotAngle;
					m_aHaloInfo[nNewHaloIndex].fIntensityFadeSpeed = 2.0f;
					m_aHaloInfo[nNewHaloIndex].bFadeEnds = TRUE;
				}
			}

			ColorRGBA.White();
			ColorRGBA.fAlpha = m_fTargetedUnitIntensity;
			_Draw_TetherPentagon( pViewport, &Origin_SS, &ColorRGBA, _PENT2_NORM_THICKNESS, fNormRadius, bRotateRing ? m_fTargetedLockRotAngle : 0.0f, bRotateRing );
		}

		// Update and draw lock rings...

		if( m_fTargetedUnitZoom == 1.0f ) {
			if( m_fTargetedUnitLock < 1.0f ) {
				Origin_SS.x = FMATH_FPOT( m_fTargetedUnitLock, m_fNormOriginX, m_TargetedPoint_SS.x );
				Origin_SS.y = FMATH_FPOT( m_fTargetedUnitLock, m_fNormOriginY, m_TargetedPoint_SS.y );
				fNormRadius = FMATH_FPOT( m_fTargetedUnitLock, _HALORING_START_RADIUS, fNormEndRadius );

				if( m_fTargetedUnitIntensity == 1.0f ) {
					m_fTargetedUnitLock += FLoop_fPreviousLoopSecs * m_fTargetLockOnSpeed;
					if( m_fTargetedUnitLock >= 1.0f ) {
						// Fully locked...
						m_fTargetedUnitLock = 1.0f;
						m_fTargetedUnitLockSweep = 0.0f;
						m_bTargetedLockPlayBeep = FALSE;
					}
				}

				ColorRGBA.fRed = 1.0f;
				ColorRGBA.fGreen = 0.2f;
				ColorRGBA.fBlue = 0.2f;
				ColorRGBA.fAlpha = m_fTargetedUnitIntensity * m_fTargetedUnitLock;
				_Draw_TetherPentagon( pViewport, &Origin_SS, &ColorRGBA, _PENT2_NORM_THICKNESS, fNormRadius, m_fTargetedLockRotAngle, FALSE );
			} else {
				// Fully locked...

				if( m_fTargetedUnitIntensity == 1.0f ) {
					fPrevVal = m_fTargetedUnitLockSweep + _LOCKED_SWEEP_OFFSET;
					m_fTargetedUnitLockSweep += FLoop_fPreviousLoopSecs * 2.0f;
					if( m_fTargetedUnitLockSweep > 1.0f ) {
						m_fTargetedUnitLockSweep = 0.0f;
					}

					fUnitStep = m_fTargetedUnitLockSweep + _LOCKED_SWEEP_OFFSET;
					if( (fPrevVal < 1.0f) && (fUnitStep <= 1.0f) ) {
						nPrevHaloIndex = (u32)(fPrevVal * ((f32)(HALO_RING_COUNT-1)));
						FASSERT( nPrevHaloIndex < HALO_RING_COUNT );
						nNewHaloIndex = (u32)(fUnitStep * ((f32)(HALO_RING_COUNT-1)));
						FASSERT( nNewHaloIndex < HALO_RING_COUNT );

						if( nNewHaloIndex != nPrevHaloIndex ) {
							m_aHaloInfo[nNewHaloIndex].ColorRGB.Green();
							m_aHaloInfo[nNewHaloIndex].fUnitIntensity = 0.4f;
							m_aHaloInfo[nNewHaloIndex].fRotAngle = 0.0f;
							m_aHaloInfo[nNewHaloIndex].fIntensityFadeSpeed = 1.0f;
							m_aHaloInfo[nNewHaloIndex].bFadeEnds = FALSE;
						}
					}
				}
			}
		}

		// Draw halos...
		for( i=0; i<HALO_RING_COUNT; ++i ) {
			if( m_aHaloInfo[i].fUnitIntensity > 0.0f ) {
				fUnitStep = (f32)i * m_fHaloRingStep;

				Origin_SS.x = FMATH_FPOT( fUnitStep, m_fNormOriginX, m_TargetedPoint_SS.x );
				Origin_SS.y = FMATH_FPOT( fUnitStep, m_fNormOriginY, m_TargetedPoint_SS.y );
				fNormRadius = FMATH_FPOT( fUnitStep, _HALORING_START_RADIUS, fNormEndRadius );

				m_aHaloInfo[i].fUnitIntensity -= FLoop_fPreviousLoopSecs * m_aHaloInfo[i].fIntensityFadeSpeed;
				FMATH_CLAMPMIN( m_aHaloInfo[i].fUnitIntensity, 0.0f );

				ColorRGBA = m_aHaloInfo[i].ColorRGB;
				ColorRGBA.fAlpha = m_fTargetedUnitIntensity * m_aHaloInfo[i].fUnitIntensity;
				_Draw_TetherPentagon( pViewport, &Origin_SS, &ColorRGBA, _PENT2_NORM_THICKNESS, fNormRadius, m_aHaloInfo[i].fRotAngle, m_aHaloInfo[i].bFadeEnds );
			}
		}

		if( m_fTargetedUnitIntensity == 1.0f ) {
			if( m_fTargetedUnitZoom > 0.0f ) {
				m_fTargetedLockRotAngle += FLoop_fPreviousLoopSecs * FMATH_2PI * 0.7f;
				while( m_fTargetedLockRotAngle > FMATH_2PI ) {
					m_fTargetedLockRotAngle -= FMATH_2PI;
				}
			}
		}
	}

	if( IsDrawEnabled() ) {
		if( m_fUnitAnimScale < 1.0f ) {
			m_fUnitAnimScale += _ANIMATE_SPEED * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fUnitAnimScale, 1.0f );
		}
	} else {
		if( m_fUnitAnimScale > 0.0f ) {
			m_fUnitAnimScale -= _ANIMATE_SPEED * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fUnitAnimScale, 0.0f );
		}
	}
}


void CReticle::_Draw_TetherLineScale( FViewport_t *pViewport, const CFColorRGBA *pColorRGBA, f32 fNormInnerRadius, f32 fNormOuterRadius, f32 fNormTickSize, f32 fScale ) {
	f32 fAnimOffset, fPosNormOuterRadius, fNegNormOuterRadius, fPosNormInnerRadius, fNegNormInnerRadius;
	f32 fNormTickOffset1, fNormTickOffset2, fNormTickSize0, fNormTickSize1, fNormTickSize2;
	u32 i;

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_SetTexture( NULL );

	for( i=0; i<8; ++i ) {
		m_aFDrawVtx[i].ColorRGBA = *pColorRGBA;
	}

	fNormInnerRadius *= fScale;
	fNormOuterRadius *= fScale;

	fAnimOffset = 0.1f * (1.0f - m_fUnitAnimScale);
	fPosNormOuterRadius =   fAnimOffset + fNormOuterRadius;
	fNegNormOuterRadius = -(fAnimOffset + fNormOuterRadius);
	fPosNormInnerRadius =   fAnimOffset + fNormInnerRadius;
	fNegNormInnerRadius = -(fAnimOffset + fNormInnerRadius);

	fNormTickSize0 = fNormTickSize * fScale;
	fNormTickSize1 = fNormTickSize * (3.0f/4.0f);
	fNormTickSize2 = fNormTickSize * (2.0f/4.0f);

	fNormTickOffset2 = (fNormOuterRadius - fNormInnerRadius) * 0.8f;
	fNormTickOffset1 = fNormTickOffset2 * 0.5f;

	// Left linescale...
	m_aFDrawVtx[0].Pos_MS.x = m_fNormOriginX + fNegNormOuterRadius;
	m_aFDrawVtx[0].Pos_MS.y = m_fNormOriginY;
	m_aFDrawVtx[1].Pos_MS.x = m_fNormOriginX + fNegNormInnerRadius;
	m_aFDrawVtx[1].Pos_MS.y = m_fNormOriginY;

	m_aFDrawVtx[2].Pos_MS.x = m_aFDrawVtx[0].Pos_MS.x;
	m_aFDrawVtx[2].Pos_MS.y = m_fNormOriginY + fNormTickSize0;
	m_aFDrawVtx[3].Pos_MS.x = m_aFDrawVtx[2].Pos_MS.x;
	m_aFDrawVtx[3].Pos_MS.y = m_fNormOriginY - fNormTickSize0;

	m_aFDrawVtx[4].Pos_MS.x = m_aFDrawVtx[0].Pos_MS.x + fNormTickOffset1;
	m_aFDrawVtx[4].Pos_MS.y = m_fNormOriginY + fNormTickSize1;
	m_aFDrawVtx[5].Pos_MS.x = m_aFDrawVtx[4].Pos_MS.x;
	m_aFDrawVtx[5].Pos_MS.y = m_fNormOriginY - fNormTickSize1;

	m_aFDrawVtx[6].Pos_MS.x = m_aFDrawVtx[0].Pos_MS.x + fNormTickOffset2;
	m_aFDrawVtx[6].Pos_MS.y = m_fNormOriginY + fNormTickSize2;
	m_aFDrawVtx[7].Pos_MS.x = m_aFDrawVtx[6].Pos_MS.x;
	m_aFDrawVtx[7].Pos_MS.y = m_fNormOriginY - fNormTickSize2;

	for( i=0; i<8; ++i ) {
		m_aFDrawVtx[i].Pos_MS.x *= pViewport->HalfRes.y;
		m_aFDrawVtx[i].Pos_MS.y *= pViewport->HalfRes.y;
	}

	fdraw_PrimList( FDRAW_PRIMTYPE_LINELIST, m_aFDrawVtx, 8 );

	// Right linescale...
	m_aFDrawVtx[0].Pos_MS.x = m_fNormOriginX + fPosNormOuterRadius;
	m_aFDrawVtx[0].Pos_MS.y = m_fNormOriginY;
	m_aFDrawVtx[1].Pos_MS.x = m_fNormOriginX + fPosNormInnerRadius;
	m_aFDrawVtx[1].Pos_MS.y = m_fNormOriginY;

	m_aFDrawVtx[2].Pos_MS.x = m_aFDrawVtx[0].Pos_MS.x;
	m_aFDrawVtx[2].Pos_MS.y = m_fNormOriginY + fNormTickSize0;
	m_aFDrawVtx[3].Pos_MS.x = m_aFDrawVtx[2].Pos_MS.x;
	m_aFDrawVtx[3].Pos_MS.y = m_fNormOriginY - fNormTickSize0;

	m_aFDrawVtx[4].Pos_MS.x = m_aFDrawVtx[0].Pos_MS.x - fNormTickOffset1;
	m_aFDrawVtx[4].Pos_MS.y = m_fNormOriginY + fNormTickSize1;
	m_aFDrawVtx[5].Pos_MS.x = m_aFDrawVtx[4].Pos_MS.x;
	m_aFDrawVtx[5].Pos_MS.y = m_fNormOriginY - fNormTickSize1;

	m_aFDrawVtx[6].Pos_MS.x = m_aFDrawVtx[0].Pos_MS.x - fNormTickOffset2;
	m_aFDrawVtx[6].Pos_MS.y = m_fNormOriginY + fNormTickSize2;
	m_aFDrawVtx[7].Pos_MS.x = m_aFDrawVtx[6].Pos_MS.x;
	m_aFDrawVtx[7].Pos_MS.y = m_fNormOriginY - fNormTickSize2;

	for( i=0; i<8; ++i ) {
		m_aFDrawVtx[i].Pos_MS.x *= pViewport->HalfRes.y;
		m_aFDrawVtx[i].Pos_MS.y *= pViewport->HalfRes.y;
	}

	fdraw_PrimList( FDRAW_PRIMTYPE_LINELIST, m_aFDrawVtx, 8 );

	// Bottom linescale...
	m_aFDrawVtx[0].Pos_MS.x = m_fNormOriginX;
	m_aFDrawVtx[0].Pos_MS.y = m_fNormOriginY + fNegNormOuterRadius;
	m_aFDrawVtx[1].Pos_MS.x = m_fNormOriginX;
	m_aFDrawVtx[1].Pos_MS.y = m_fNormOriginY + fNegNormInnerRadius;

	m_aFDrawVtx[2].Pos_MS.x = m_fNormOriginX + fNormTickSize0;
	m_aFDrawVtx[2].Pos_MS.y = m_aFDrawVtx[0].Pos_MS.y;
	m_aFDrawVtx[3].Pos_MS.x = m_fNormOriginX - fNormTickSize0;
	m_aFDrawVtx[3].Pos_MS.y = m_aFDrawVtx[2].Pos_MS.y;

	m_aFDrawVtx[4].Pos_MS.x = m_fNormOriginX + fNormTickSize1;
	m_aFDrawVtx[4].Pos_MS.y = m_aFDrawVtx[0].Pos_MS.y + fNormTickOffset1;
	m_aFDrawVtx[5].Pos_MS.x = m_fNormOriginX - fNormTickSize1;
	m_aFDrawVtx[5].Pos_MS.y = m_aFDrawVtx[4].Pos_MS.y;

	m_aFDrawVtx[6].Pos_MS.x = m_fNormOriginX + fNormTickSize2;
	m_aFDrawVtx[6].Pos_MS.y = m_aFDrawVtx[0].Pos_MS.y + fNormTickOffset2;
	m_aFDrawVtx[7].Pos_MS.x = m_fNormOriginX - fNormTickSize2;
	m_aFDrawVtx[7].Pos_MS.y = m_aFDrawVtx[6].Pos_MS.y;

	for( i=0; i<8; ++i ) {
		m_aFDrawVtx[i].Pos_MS.x *= pViewport->HalfRes.y;
		m_aFDrawVtx[i].Pos_MS.y *= pViewport->HalfRes.y;
	}

	fdraw_PrimList( FDRAW_PRIMTYPE_LINELIST, m_aFDrawVtx, 8 );

	// Top linescale...
	m_aFDrawVtx[0].Pos_MS.x = m_fNormOriginX;
	m_aFDrawVtx[0].Pos_MS.y = m_fNormOriginY + fPosNormOuterRadius;
	m_aFDrawVtx[1].Pos_MS.x = m_fNormOriginX;
	m_aFDrawVtx[1].Pos_MS.y = m_fNormOriginY + fPosNormInnerRadius;

	m_aFDrawVtx[2].Pos_MS.x = m_fNormOriginX + fNormTickSize0;
	m_aFDrawVtx[2].Pos_MS.y = m_aFDrawVtx[0].Pos_MS.y;
	m_aFDrawVtx[3].Pos_MS.x = m_fNormOriginX - fNormTickSize0;
	m_aFDrawVtx[3].Pos_MS.y = m_aFDrawVtx[2].Pos_MS.y;

	m_aFDrawVtx[4].Pos_MS.x = m_fNormOriginX + fNormTickSize1;
	m_aFDrawVtx[4].Pos_MS.y = m_aFDrawVtx[0].Pos_MS.y - fNormTickOffset1;
	m_aFDrawVtx[5].Pos_MS.x = m_fNormOriginX - fNormTickSize1;
	m_aFDrawVtx[5].Pos_MS.y = m_aFDrawVtx[4].Pos_MS.y;

	m_aFDrawVtx[6].Pos_MS.x = m_fNormOriginX + fNormTickSize2;
	m_aFDrawVtx[6].Pos_MS.y = m_aFDrawVtx[0].Pos_MS.y - fNormTickOffset2;
	m_aFDrawVtx[7].Pos_MS.x = m_fNormOriginX - fNormTickSize2;
	m_aFDrawVtx[7].Pos_MS.y = m_aFDrawVtx[6].Pos_MS.y;

	for( i=0; i<8; ++i ) {
		m_aFDrawVtx[i].Pos_MS.x *= pViewport->HalfRes.y;
		m_aFDrawVtx[i].Pos_MS.y *= pViewport->HalfRes.y;
	}

	fdraw_PrimList( FDRAW_PRIMTYPE_LINELIST, m_aFDrawVtx, 8 );
}

void CReticle::_Draw_Scope( void ) {
	
	if( m_aTexInst[ TEXTURE_DROID_SCOPE1 ].GetTexDef() == NULL ) {
		return;
	}

	if( m_aTexInst[ TEXTURE_DROID_SCOPE2 ].GetTexDef() == NULL ) {
		return;
	}

	CFXfm::InitStack();

	int nPlayer = this->m_nPlayerIndex;
	FViewport_t *pViewport = Player_aPlayer[nPlayer].m_pViewportOrtho3D;
	fviewport_SetActive( pViewport );

	frenderer_Push( FRENDERER_DRAW, NULL );

	fdraw_ClearDepth_Cache();

	// Render mask into depth buffer...
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_DST );
	fdraw_Alpha_SetTest( FDRAW_ALPHATEST_GEQUAL, 0.5f );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECALTEX_AT );
	fdraw_Depth_EnableWriting( TRUE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );

	CFColorRGBA ColorBias;
	ColorBias.OpaqueWhite();
	_Draw_DroidScopeTexture( &m_aTexInst[TEXTURE_DROID_SCOPE2], &ColorBias );

	// Render static outside reticle...
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECALTEX_AI_INTENSITY );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER );

	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	ColorBias.Set( 0.3f, 0.4f, 0.3f, 0.7f );
	_Draw_ScopeStatic( &ColorBias );

	// Render scope HUD and gadgets...
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	ColorBias.Set( 0.1f, 0.7f, 1.0f, 1.0f );
	_Draw_DroidScopeTexture( &m_aTexInst[TEXTURE_DROID_SCOPE1], &ColorBias );
	_Draw_ScopeGadgets();

	frenderer_Pop();

	// Remove our mask from the depth buffer...
//	fviewport_Clear( FVIEWPORT_CLEARFLAG_DEPTH, 0.0f, 0.0f, 0.0f, 1.0f, 0 );
}

void CReticle::_Draw_TetherPentagon( FViewport_t *pViewport, const CFVec3A *pNormOrig, const CFColorRGBA *pColorRGBA, f32 fNormThickness, f32 fNormRadius, f32 fRotAngle, BOOL bFadeEnds ) {
	f32 fMinNormRadius, fNormShortSideHalfLength, fNormLongSideHalfLength;
	u32 i, j;
	CFVec3A aCorners[4];

	for( i=0; i<4; ++i ) {
		m_aFDrawVtx[i].ColorRGBA = *pColorRGBA;
	}

	if( bFadeEnds ) {
		m_aFDrawVtx[0].ColorRGBA.fAlpha *= 0.3f;
		m_aFDrawVtx[1].ColorRGBA.fAlpha *= 0.3f;
	}

	fMinNormRadius = 2.0f * _PENT2_NORM_GAP * (1.0f / 0.725f);
	FMATH_CLAMPMIN( fNormRadius, fMinNormRadius );

	fNormShortSideHalfLength = 0.725f*fNormRadius - _PENT2_NORM_GAP;
	fNormLongSideHalfLength = fNormShortSideHalfLength + m_fPentVtxNormOffset;

	aCorners[0].x = -fNormShortSideHalfLength;
	aCorners[0].y = -fNormRadius;

	aCorners[1].x = -fNormLongSideHalfLength;
	aCorners[1].y = -fNormRadius - fNormThickness;

	aCorners[2].x = fNormShortSideHalfLength;
	aCorners[2].y = aCorners[0].y;

	aCorners[3].x = fNormLongSideHalfLength;
	aCorners[3].y = aCorners[1].y;

	aCorners[0].RotateZ( fRotAngle );
	aCorners[1].RotateZ( fRotAngle );
	aCorners[2].RotateZ( fRotAngle );
	aCorners[3].RotateZ( fRotAngle );

	// Each side...
	for( j=0; j<5; ++j ) {
		if( j ) {
			aCorners[0].RotateZ( m_fPentSideSin, m_fPentSideCos );
			aCorners[1].RotateZ( m_fPentSideSin, m_fPentSideCos );
			aCorners[2].RotateZ( m_fPentSideSin, m_fPentSideCos );
			aCorners[3].RotateZ( m_fPentSideSin, m_fPentSideCos );
		}

		for( i=0; i<4; ++i ) {
			m_aFDrawVtx[i].Pos_MS.x = pViewport->HalfRes.y * aCorners[i].x + pViewport->HalfRes.x * pNormOrig->x;
			m_aFDrawVtx[i].Pos_MS.y = pViewport->HalfRes.y * (aCorners[i].y + pNormOrig->y);
		}

		fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, m_aFDrawVtx, 4 );
	}
}


#define _DROIDSCOPE_TC_LEFT_X		0.4016f
#define _DROIDSCOPE_TC_RIGHT_X		1.0000f
#define _DROIDSCOPE_TC_TOP_Y		0.1429f
#define _DROIDSCOPE_TC_BOTTOM_Y		1.0000f

#define _DROIDSCOPE_TC_WIDTH		(_DROIDSCOPE_TC_RIGHT_X - _DROIDSCOPE_TC_LEFT_X)
#define _DROIDSCOPE_TC_HEIGHT		(_DROIDSCOPE_TC_BOTTOM_Y - _DROIDSCOPE_TC_TOP_Y)


void CReticle::_Draw_DroidScopeTexture( CFTexInst *pTexInst, const CFColorRGBA *pColorBias ) {
	FDrawVtx_t aDrawVtx[4];
	f32 fZ, fNormScaleX, fNormScaleY;
	u32 i;

	FViewport_t *pViewport = fviewport_GetActive();

	fdraw_SetTexture( pTexInst );

	fNormScaleY = m_fNormScale * 0.4f;
	fNormScaleX = fNormScaleY * fmath_Div(
									(f32)pTexInst->GetTexDef()->TexInfo.nTexelsAcross * _DROIDSCOPE_TC_WIDTH,
									(f32)pTexInst->GetTexDef()->TexInfo.nTexelsDown * _DROIDSCOPE_TC_HEIGHT
								);

	fZ = 0.5f * (pViewport->fNearZ + pViewport->fFarZ);

	for( i=0; i<4; ++i ) {
		aDrawVtx[i].Pos_MS.z = fZ;
		aDrawVtx[i].ColorRGBA = *pColorBias;
	}

	aDrawVtx[0].ST.Set( _DROIDSCOPE_TC_LEFT_X, _DROIDSCOPE_TC_TOP_Y );
	aDrawVtx[1].ST.Set( _DROIDSCOPE_TC_LEFT_X, _DROIDSCOPE_TC_BOTTOM_Y );
	aDrawVtx[2].ST.Set( _DROIDSCOPE_TC_RIGHT_X, _DROIDSCOPE_TC_TOP_Y );
	aDrawVtx[3].ST.Set( _DROIDSCOPE_TC_RIGHT_X, _DROIDSCOPE_TC_BOTTOM_Y );

	// Upper-left tile...
	aDrawVtx[0].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX - fNormScaleX);
	aDrawVtx[0].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY + fNormScaleY);

	aDrawVtx[1].Pos_MS.x = aDrawVtx[0].Pos_MS.x;
	aDrawVtx[1].Pos_MS.y = pViewport->HalfRes.y * m_fNormOriginY;

	aDrawVtx[2].Pos_MS.x = pViewport->HalfRes.y * m_fNormOriginX;
	aDrawVtx[2].Pos_MS.y = aDrawVtx[0].Pos_MS.y;

	aDrawVtx[3].Pos_MS.x = aDrawVtx[2].Pos_MS.x;
	aDrawVtx[3].Pos_MS.y = aDrawVtx[1].Pos_MS.y;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aDrawVtx, 4 );

	// Upper-right tile...
	aDrawVtx[0].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX + fNormScaleX);
	aDrawVtx[0].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY + fNormScaleY);

	aDrawVtx[1].Pos_MS.x = aDrawVtx[0].Pos_MS.x;
	aDrawVtx[1].Pos_MS.y = pViewport->HalfRes.y * m_fNormOriginY;

	aDrawVtx[2].Pos_MS.x = pViewport->HalfRes.y * m_fNormOriginX;
	aDrawVtx[2].Pos_MS.y = aDrawVtx[0].Pos_MS.y;

	aDrawVtx[3].Pos_MS.x = aDrawVtx[2].Pos_MS.x;
	aDrawVtx[3].Pos_MS.y = aDrawVtx[1].Pos_MS.y;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aDrawVtx, 4 );

	// Lower-left tile...
	aDrawVtx[0].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX - fNormScaleX);
	aDrawVtx[0].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY - fNormScaleY);

	aDrawVtx[1].Pos_MS.x = aDrawVtx[0].Pos_MS.x;
	aDrawVtx[1].Pos_MS.y = pViewport->HalfRes.y * m_fNormOriginY;

	aDrawVtx[2].Pos_MS.x = pViewport->HalfRes.y * m_fNormOriginX;
	aDrawVtx[2].Pos_MS.y = aDrawVtx[0].Pos_MS.y;

	aDrawVtx[3].Pos_MS.x = aDrawVtx[2].Pos_MS.x;
	aDrawVtx[3].Pos_MS.y = aDrawVtx[1].Pos_MS.y;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aDrawVtx, 4 );

	// Lower-right tile...
	aDrawVtx[0].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX + fNormScaleX);
	aDrawVtx[0].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY - fNormScaleY);

	aDrawVtx[1].Pos_MS.x = aDrawVtx[0].Pos_MS.x;
	aDrawVtx[1].Pos_MS.y = pViewport->HalfRes.y * m_fNormOriginY;

	aDrawVtx[2].Pos_MS.x = pViewport->HalfRes.y * m_fNormOriginX;
	aDrawVtx[2].Pos_MS.y = aDrawVtx[0].Pos_MS.y;

	aDrawVtx[3].Pos_MS.x = aDrawVtx[2].Pos_MS.x;
	aDrawVtx[3].Pos_MS.y = aDrawVtx[1].Pos_MS.y;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aDrawVtx, 4 );
}


#define	_STATIC_TEXTURE_SCALE_X		0.2f
#define	_STATIC_TEXTURE_SCALE_Y		1.5f


void CReticle::_Draw_ScopeStatic( const CFColorRGBA *pColorBias ) {
	FDrawVtx_t aDrawVtx[4];
	f32 fTexOffsetX, fTexOffsetY, fZ;
	u32 i;

	FViewport_t *pViewport = fviewport_GetActive();

	fdraw_SetTexture( &m_aTexInst[TEXTURE_STATIC] );

	fZ = 0.25f * pViewport->fNearZ + 0.75f * pViewport->fFarZ;

	aDrawVtx[0].Pos_MS.Set( -pViewport->HalfRes.x,  pViewport->HalfRes.y, fZ );
	aDrawVtx[1].Pos_MS.Set( -pViewport->HalfRes.x, -pViewport->HalfRes.y, fZ );
	aDrawVtx[2].Pos_MS.Set(  pViewport->HalfRes.x,  pViewport->HalfRes.y, fZ );
	aDrawVtx[3].Pos_MS.Set(  pViewport->HalfRes.x, -pViewport->HalfRes.y, fZ );

	for( i=0; i<4; ++i ) {
		aDrawVtx[i].ColorRGBA = *pColorBias;
	}

	fTexOffsetX = fmath_RandomFloat() * _STATIC_TEXTURE_SCALE_X;
	fTexOffsetY = fmath_RandomFloat() * _STATIC_TEXTURE_SCALE_Y;

	// For multiplayer, make the texture bigger
	float fSizeX = _STATIC_TEXTURE_SCALE_X;
	float fSizeY = _STATIC_TEXTURE_SCALE_Y;
	if (CPlayer::m_nPlayerCount > 1) {
		fSizeX *= 0.5f;
		fSizeY *= 0.5f;
	}
	
	aDrawVtx[0].ST.Set( fTexOffsetX, fTexOffsetY );
	aDrawVtx[1].ST.Set( fTexOffsetX, fTexOffsetY + fSizeY );
	aDrawVtx[2].ST.Set( fTexOffsetX + fSizeX, fTexOffsetY );
	aDrawVtx[3].ST.Set( fTexOffsetX + fSizeX, fTexOffsetY + fSizeY );

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aDrawVtx, 4 );
}


#define _DROIDSCOPE_TC_RET_LEFT_X		0.0787f
#define _DROIDSCOPE_TC_RET_RIGHT_X		0.2204f
#define _DROIDSCOPE_TC_RET_TOP_Y		0.7460f
#define _DROIDSCOPE_TC_RET_BOTTOM_Y		1.0000f

#define _DROIDSCOPE_TC_TIC_LEFT_X		0.0000f
#define _DROIDSCOPE_TC_TIC_RIGHT_X		0.0472f
#define _DROIDSCOPE_TC_TIC_TOP_Y		0.0000f
#define _DROIDSCOPE_TC_TIC_BOTTOM_Y		1.0000f

#define _DROIDSCOPE_TC_TIC_WIDTH		(_DROIDSCOPE_TC_TIC_RIGHT_X - _DROIDSCOPE_TC_TIC_LEFT_X)
#define _DROIDSCOPE_TC_TIC_HEIGHT		(_DROIDSCOPE_TC_TIC_BOTTOM_Y - _DROIDSCOPE_TC_TIC_TOP_Y)

#define _SCOPE_INFO_TBOX_WIDTH			0.23f
#define _SCOPE_INFO_TBOX_HEIGHT			0.024f



void CReticle::_Draw_ScopeGadgets( void ) {
	FDrawVtx_t aDrawVtx[4];
	f32 fNormScaleX, fNormScaleY, fTCOffset, fTextOffsetY;
	u32 i;
	CPlayer *pPlayer = &Player_aPlayer[ m_nPlayerIndex ];

	FViewport_t *pViewport = fviewport_GetActive();
	fdraw_SetTexture( &m_aTexInst[TEXTURE_DROID_SCOPE1] );

	// Draw reticle...
	fNormScaleY = m_fNormScale * 0.03f;
	fNormScaleX = fNormScaleY;

	if( !(m_nFlags & FLAG_ACTIVE_COLOR) ) {
		// Inactive color...

		for( i=0; i<4; ++i ) {
			aDrawVtx[i].ColorRGBA = m_InactiveColor;
			aDrawVtx[i].Pos_MS.z = 100.0f;
		}
	} else {
		// Active color...

		for( i=0; i<4; ++i ) {
			aDrawVtx[i].ColorRGBA = m_ActiveColor;
			aDrawVtx[i].Pos_MS.z = 100.0f;
		}
	}

	aDrawVtx[0].ST.Set( _DROIDSCOPE_TC_RET_LEFT_X, _DROIDSCOPE_TC_RET_TOP_Y );
	aDrawVtx[1].ST.Set( _DROIDSCOPE_TC_RET_LEFT_X, _DROIDSCOPE_TC_RET_BOTTOM_Y );
	aDrawVtx[2].ST.Set( _DROIDSCOPE_TC_RET_RIGHT_X, _DROIDSCOPE_TC_RET_TOP_Y );
	aDrawVtx[3].ST.Set( _DROIDSCOPE_TC_RET_RIGHT_X, _DROIDSCOPE_TC_RET_BOTTOM_Y );

	aDrawVtx[0].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX - fNormScaleX);
	aDrawVtx[0].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY + fNormScaleY);

	aDrawVtx[1].Pos_MS.x = aDrawVtx[0].Pos_MS.x;
	aDrawVtx[1].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY - fNormScaleY);

	aDrawVtx[2].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX + fNormScaleX);
	aDrawVtx[2].Pos_MS.y = aDrawVtx[0].Pos_MS.y;

	aDrawVtx[3].Pos_MS.x = aDrawVtx[2].Pos_MS.x;
	aDrawVtx[3].Pos_MS.y = aDrawVtx[1].Pos_MS.y;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aDrawVtx, 4 );

	// Draw horizontal ticks...
	for( i=0; i<4; ++i ) {
		aDrawVtx[i].ColorRGBA.Set( 0.6f, 0.7f, 0.7f, 1.0f );
	}

	fNormScaleX = m_fNormScale * 0.35f;
	fNormScaleY = m_fNormScale * 0.025f;

	fTCOffset = m_fScopeHeading * (8.0f / FMATH_2PI);

	aDrawVtx[0].ST.Set( _DROIDSCOPE_TC_TIC_LEFT_X, _DROIDSCOPE_TC_TIC_BOTTOM_Y + fTCOffset );
	aDrawVtx[1].ST.Set( _DROIDSCOPE_TC_TIC_RIGHT_X, _DROIDSCOPE_TC_TIC_BOTTOM_Y + fTCOffset );
	aDrawVtx[2].ST.Set( _DROIDSCOPE_TC_TIC_LEFT_X, _DROIDSCOPE_TC_TIC_TOP_Y + fTCOffset );
	aDrawVtx[3].ST.Set( _DROIDSCOPE_TC_TIC_RIGHT_X, _DROIDSCOPE_TC_TIC_TOP_Y + fTCOffset );

	aDrawVtx[0].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX - fNormScaleX);
	aDrawVtx[0].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY + fNormScaleY - 0.45f*m_fNormScale);

	aDrawVtx[1].Pos_MS.x = aDrawVtx[0].Pos_MS.x;
	aDrawVtx[1].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY - fNormScaleY - 0.45f*m_fNormScale);

	aDrawVtx[2].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX + fNormScaleX);
	aDrawVtx[2].Pos_MS.y = aDrawVtx[0].Pos_MS.y;

	aDrawVtx[3].Pos_MS.x = aDrawVtx[2].Pos_MS.x;
	aDrawVtx[3].Pos_MS.y = aDrawVtx[1].Pos_MS.y;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aDrawVtx, 4 );

	// Draw vertical ticks...
	for( i=0; i<4; ++i ) {
		aDrawVtx[i].ColorRGBA.Set( 0.6f, 0.7f, 0.7f, 1.0f );
	}

	fNormScaleX = m_fNormScale * 0.025f;
	fNormScaleY = m_fNormScale * 0.20f;

	fTCOffset = m_fScopePitch * (8.0f / FMATH_2PI);

	aDrawVtx[0].ST.Set( _DROIDSCOPE_TC_TIC_LEFT_X, (_DROIDSCOPE_TC_TIC_TOP_Y - fTCOffset) * 0.6f );
	aDrawVtx[1].ST.Set( _DROIDSCOPE_TC_TIC_LEFT_X, (_DROIDSCOPE_TC_TIC_BOTTOM_Y - fTCOffset) * 0.6f );
	aDrawVtx[2].ST.Set( _DROIDSCOPE_TC_TIC_RIGHT_X, (_DROIDSCOPE_TC_TIC_TOP_Y - fTCOffset) * 0.6f );
	aDrawVtx[3].ST.Set( _DROIDSCOPE_TC_TIC_RIGHT_X, (_DROIDSCOPE_TC_TIC_BOTTOM_Y - fTCOffset) * 0.6f );

	aDrawVtx[0].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX + fNormScaleX - 0.60f*m_fNormScale);
	aDrawVtx[0].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY + fNormScaleY);

	aDrawVtx[1].Pos_MS.x = aDrawVtx[0].Pos_MS.x;
	aDrawVtx[1].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY - fNormScaleY);

	aDrawVtx[2].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX - fNormScaleX - 0.60f*m_fNormScale);
	aDrawVtx[2].Pos_MS.y = aDrawVtx[0].Pos_MS.y;

	aDrawVtx[3].Pos_MS.x = aDrawVtx[2].Pos_MS.x;
	aDrawVtx[3].Pos_MS.y = aDrawVtx[1].Pos_MS.y;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aDrawVtx, 4 );

	// Draw zoom magnification text...
	FTextArea_t *pTextArea = ftext_GetAttributes( pPlayer->m_hTextBoxScopeZoom );

	if( pTextArea->bVisible && !(m_nScopeFlashMagCount & 1) ) {
		pTextArea->fUpperLeftX = 0.5f + m_fNormScale * (0.23f - 0.5f*_SCOPE_ZOOM_TBOX_WIDTH);
		pTextArea->fLowerRightX = 0.5f + m_fNormScale * (0.23f + 0.5f*_SCOPE_ZOOM_TBOX_WIDTH);
		pTextArea->fUpperLeftY = 0.5f + m_fNormScale * (0.013f - 0.5f*_SCOPE_ZOOM_TBOX_HEIGHT);
		pTextArea->fLowerRightY = 0.5f + m_fNormScale * (0.013f + 0.5f*_SCOPE_ZOOM_TBOX_HEIGHT);

		u32 nZoomMag = fmath_FloatToU32( m_fScopeZoomMultiplier );
		FMATH_CLAMPMAX( nZoomMag, 99 );

		if( nZoomMag < 10 ) {
			_szScopeZoomString[13] = (char)nZoomMag + '0';
			_szScopeZoomString[14] = 0;
		} else {
			_szScopeZoomString[13] = (char)(nZoomMag / 10) + '0';
			_szScopeZoomString[14] = (char)(nZoomMag % 10) + '0';
			_szScopeZoomString[15] = 0;
		}

		ftext_PrintString( pPlayer->m_hTextBoxScopeZoom, _szScopeZoomString );
	}

	// Draw target info text...
	if( m_pScopeTargetedBot && (m_nFlags & FLAG_DISPLAY_SCOPE_TARGET_INFO) ) {
		fTextOffsetY = -0.20f;

		for( i=0; i<RETICLE_SCOPE_INFO_SLOT_COUNT; ++i, fTextOffsetY += 0.03f ) {
			pTextArea = ftext_GetAttributes( pPlayer->m_ahTextBoxScopeInfo[i] );

			pTextArea->fUpperLeftX = 0.5f + m_fNormScale * (0.325f - 0.5f*_SCOPE_INFO_TBOX_WIDTH);
			pTextArea->fLowerRightX = 0.5f + m_fNormScale * (0.325f + 0.5f*_SCOPE_INFO_TBOX_WIDTH);
			pTextArea->fUpperLeftY = 0.5f + m_fNormScale * (fTextOffsetY - 0.5f*_SCOPE_INFO_TBOX_HEIGHT);
			pTextArea->fLowerRightY = 0.5f + m_fNormScale * (fTextOffsetY + 0.5f*_SCOPE_INFO_TBOX_HEIGHT);

			pTextArea->oColorForeground.fRed = FMATH_FPOT( m_afScopeInfoIntensity[i], 0.2f, 1.0f );
			pTextArea->oColorForeground.fGreen = FMATH_FPOT( m_afScopeInfoIntensity[i], 0.75f, 1.0f );
			pTextArea->oColorForeground.fBlue = FMATH_FPOT( m_afScopeInfoIntensity[i], 1.0f, 1.0f );
			pTextArea->oColorForeground.fAlpha = 1.0f;

			if( (i != (m_nScopeInfoDisplayCount - 1)) || (m_pwszScopeTargetName == NULL) ) {
				// Don't draw text box border...
				pTextArea->oColorBorder.fAlpha = 0.0f;
			} else {
				// Draw text box border...
				pTextArea->oColorBorder.fAlpha = 1.0f;
			}
		}

		if( (m_nScopeInfoDisplayCount >= 1) && ftext_GetAttributes( pPlayer->m_ahTextBoxScopeInfo[0] )->bVisible ) {
			// Display target name...

			if( (m_pwszScopeTargetName == NULL) || m_pScopeTargetedBot->IsTorsoBlownOff() ) {
				ftext_PrintString( pPlayer->m_ahTextBoxScopeInfo[0], Game_apwszPhrases[ GAMEPHRASE_TARGET_UNKNOWN ] );
			} else {
				FASSERT_MSG( fclib_wcslen(m_pwszScopeTargetName) <= _SCOPE_INFO_FIELD_CHAR_COUNT, "Target name exceeds max char count." );
				ftext_Printf( pPlayer->m_ahTextBoxScopeInfo[0], L"%ls %ls", Game_apwszPhrases[ GAMEPHRASE_TARGET ], m_pwszScopeTargetName );

				if( m_nScopeInfoDisplayCount >= 2 ) {
					// Display target height...
					if( (m_pScopeTargetedBot->m_fCollCylinderHeight_WS > 0.0f) && (m_pScopeTargetedBot->m_fCollCylinderHeight_WS <= 999.0f) ) {
						ftext_Printf( pPlayer->m_ahTextBoxScopeInfo[1], L"%ls %u", Game_apwszPhrases[ GAMEPHRASE_HEIGHT ], fmath_FloatToU32( m_pScopeTargetedBot->m_fCollCylinderHeight_WS ) );
					} else {
						ftext_PrintString( pPlayer->m_ahTextBoxScopeInfo[1], Game_apwszPhrases[ GAMEPHRASE_HEIGHT_UNKNOWN ] );
					}

					if( m_nScopeInfoDisplayCount >= 3 ) {
						// Display target speed...
						if( (m_pScopeTargetedBot->m_fSpeed_WS >= 0.0f) && (m_pScopeTargetedBot->m_fSpeed_WS <= 999.0f) ) {
							ftext_Printf( pPlayer->m_ahTextBoxScopeInfo[2], L"%ls %u", Game_apwszPhrases[ GAMEPHRASE_SPEED ], fmath_FloatToU32( m_pScopeTargetedBot->m_fSpeed_WS ) );
						} else {
							ftext_PrintString( pPlayer->m_ahTextBoxScopeInfo[2], Game_apwszPhrases[ GAMEPHRASE_SPEED_UNKNOWN ] );
						}

						if( m_nScopeInfoDisplayCount >= 4 ) {
							// Display target health...
							if( (m_pScopeTargetedBot->NormHealth() >= 0.0f) && (m_pScopeTargetedBot->NormHealth() <= 9.0f) ) {
								ftext_Printf( pPlayer->m_ahTextBoxScopeInfo[3], L"%ls %u%%", Game_apwszPhrases[ GAMEPHRASE_HEALTH ], fmath_FloatToU32( m_pScopeTargetedBot->NormHealth() * 100.0f ) );
							} else {
								ftext_PrintString( pPlayer->m_ahTextBoxScopeInfo[3], Game_apwszPhrases[ GAMEPHRASE_HEALTH_UNKNOWN ] );
							}

							if( m_nScopeInfoDisplayCount >= 5 ) {
								// Display data port info...
								if( m_pScopeTargetedBot->DataPort_IsInstalled() ) {
									if( m_pScopeTargetedBot->DataPort_IsOpen() ) {
										ftext_PrintString( pPlayer->m_ahTextBoxScopeInfo[4], Game_apwszPhrases[ GAMEPHRASE_D_PORT_OPEN ] );
									} else {
										ftext_PrintString( pPlayer->m_ahTextBoxScopeInfo[4], Game_apwszPhrases[ GAMEPHRASE_D_PORT_CLOSED ] );
									}
								} else {
									ftext_PrintString( pPlayer->m_ahTextBoxScopeInfo[4], Game_apwszPhrases[ GAMEPHRASE_D_PORT_NONE ] );
								}

								if( m_nScopeInfoDisplayCount >= 6 ) {
									// Display target range...
									f32 fDistToTarget = pPlayer->m_pEntityCurrent->MtxToWorld()->m_vPos.Dist( m_pScopeTargetedBot->MtxToWorld()->m_vPos );

									if( (fDistToTarget >= 0.0f) && (fDistToTarget <= 999.0f) ) {
										ftext_Printf( pPlayer->m_ahTextBoxScopeInfo[5], L"%ls %u", Game_apwszPhrases[ GAMEPHRASE_RANGE ], fmath_FloatToU32( fDistToTarget ) ); 
									} else {
										ftext_PrintString( pPlayer->m_ahTextBoxScopeInfo[5], Game_apwszPhrases[ GAMEPHRASE_RANGE_UNKNOWN ] );
									}
								}
							}
						}
					}
				}
			}
		}
	}
}


void CReticle::_ScopeFullscreenRenderTargetReadyCallback( void ) {
	if( m_aTexInst[ TEXTURE_DROID_SCOPE1 ].GetTexDef() == NULL ) {
		return;
	}

	if( m_aTexInst[ TEXTURE_DROID_SCOPE2 ].GetTexDef() == NULL ) {
		return;
	}

	FViewport_t *pViewport;
	FDrawVtx_t aVtx[4];
	CFColorRGBA ColorBias;
	f32 fDepth, fOffset, fOffset2, fDelta;
	u32 i;

	int nPlayer = CPlayer::m_nCurrent;
	CReticle *pReticle = &Player_aPlayer[nPlayer].m_Reticle;

	// If the reticle for this player is not a scope, we are probably in
	// multiplayer and drawing a viewport without a scope.
	if ( pReticle->m_nType != TYPE_DROID_SCOPE )
		return;

	CFXfm::InitStack();
	pViewport = Player_aPlayer[nPlayer].m_pViewportOrtho3D;
	fviewport_SetActive( pViewport );

	frenderer_Push( FRENDERER_DRAW, NULL );

#if	!_MMI_TARGET_PS2	//ARG - TEST
	// Render mask into depth buffer...
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_DST );
	fdraw_Alpha_SetTest( FDRAW_ALPHATEST_GEQUAL, 0.5f );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECALTEX_AT );
	fdraw_Depth_EnableWriting( TRUE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	ColorBias.OpaqueWhite();
	pReticle->_Draw_DroidScopeTexture( &m_aTexInst[TEXTURE_DROID_SCOPE2], &ColorBias );

	// Set up fdraw vertices...
	aVtx[0].ST.Set( 0.0f, 0.0f );
	aVtx[1].ST.Set( 0.0f, 1.0f );
	aVtx[2].ST.Set( 1.0f, 0.0f );
	aVtx[3].ST.Set( 1.0f, 1.0f );

	fDepth = 0.25f * pViewport->fNearZ + 0.75f * pViewport->fFarZ;

	aVtx[0].Pos_MS.Set( -pViewport->HalfRes.x,  pViewport->HalfRes.y, fDepth );
	aVtx[1].Pos_MS.Set( -pViewport->HalfRes.x, -pViewport->HalfRes.y, fDepth );
	aVtx[2].Pos_MS.Set(  pViewport->HalfRes.x,  pViewport->HalfRes.y, fDepth );
	aVtx[3].Pos_MS.Set(  pViewport->HalfRes.x, -pViewport->HalfRes.y, fDepth );

	// Render full-color image inside reticle...
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_ALPHA_TIMES_SRC );
	fdraw_Alpha_SetTest( FDRAW_ALPHATEST_ALWAYS, 0.0f );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AI );
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_FARTHER );
	fdraw_SetTexture( Game_pFullscreenRenderTarget );

	for( i=0; i<4; ++i ) {
		aVtx[i].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, 1.0f );
	}

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aVtx, 4 );

#if 0
	// Render static inside reticle...
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	ColorBias.Set( 1.0f, 1.0f, 1.0f, 0.1f );
	pReticle->_Draw_ScopeStatic( &ColorBias );
#endif

	// Render black-and-white image outside reticle...
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_ALPHA_TIMES_SRC );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECALTEX_AI_INTENSITY );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER );
	fdraw_SetTexture( Game_pFullscreenRenderTarget );

	for( i=0; i<4; ++i ) {
		aVtx[i].ColorRGBA.Set( 0.5f, 0.6f, 0.5f, 0.25f );
	}

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aVtx, 4 );

	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST );
	fDelta = FMATH_FPOT( pReticle->m_fScopeUnitJolt, 0.007f, 0.010f );

	for( i=0, fOffset=fDelta, fOffset2=1.0f-fDelta; i<(4-1); ++i, fOffset+=fDelta, fOffset2-=fDelta ) {
		aVtx[0].ST.Set( fOffset, fOffset );
		aVtx[1].ST.Set( fOffset, fOffset2 );
		aVtx[2].ST.Set( fOffset2, fOffset );
		aVtx[3].ST.Set( fOffset2, fOffset2 );

		fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aVtx, 4 );
	}

	// Render static outside reticle...
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	ColorBias.Set( 0.5f, 0.6f, 0.5f, FMATH_FPOT( pReticle->m_fScopeUnitJolt, 0.2f, 0.4f ) );
	pReticle->_Draw_ScopeStatic( &ColorBias );
#endif	//ARG - TEST

	// Render scope HUD and gadgets...
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	ColorBias.Set( 0.1f, 0.7f, 1.0f, 1.0f );
	pReticle->_Draw_DroidScopeTexture( &m_aTexInst[TEXTURE_DROID_SCOPE1], &ColorBias );
	pReticle->_Draw_ScopeGadgets();

	frenderer_Pop();

	// Remove our mask from the depth buffer...
	fviewport_Clear( FVIEWPORT_CLEARFLAG_DEPTH, 0.0f, 0.0f, 0.0f, 1.0f, 0 );
}


void CReticle::Scope_SetParameters( f32 fHeading, f32 fPitch, f32 fZoomMultiplier, CEntity *pTargetedEntity, BOOL bDisplayTargetInfo, f32 fUnitJolt ) {
	FTextArea_t *pTextArea;
	u32 i;

	FASSERT( IsCreated() );

	if( pTargetedEntity && (pTargetedEntity->TypeBits() & ENTITY_BIT_WEAPON) ) {
		// Targeting a weapon. Let's see if it belongs to a bot...

		CWeapon *pWeapon = (CWeapon *)pTargetedEntity;

		if( pWeapon->GetOwner() ) {
			// This weapon belongs to a bot...

			pTargetedEntity = pWeapon->GetOwner();
		}
	}

	m_fScopeHeading = fHeading;
	m_fScopePitch = fPitch;
	m_fScopeZoomMultiplier = fZoomMultiplier;

	if( fUnitJolt > m_fScopeUnitJolt ) {
		m_fScopeUnitJolt = fUnitJolt;
	}

	if( bDisplayTargetInfo ) {
		FMATH_SETBITMASK( m_nFlags, FLAG_DISPLAY_SCOPE_TARGET_INFO );
	} else {
		FMATH_CLEARBITMASK( m_nFlags, FLAG_DISPLAY_SCOPE_TARGET_INFO );
	}

	if( (pTargetedEntity==NULL) || !(pTargetedEntity->TypeBits() & ENTITY_BIT_BOT) ) {
		// Not targeting a bot...

		if( m_pScopeTargetedBot ) {
			m_pScopeTargetedBot = NULL;
			m_pwszScopeTargetName = NULL;
			m_fScopeInfoDisplaySecs = 0.0f;
			m_nScopeInfoDisplayCount = 0;
			m_nScopeInfoBorderIndex = 0;
			m_fScopeInfoBorderSecs = 0.0f;

			for( i=0; i<RETICLE_SCOPE_INFO_SLOT_COUNT; ++i ) {
				pTextArea = ftext_GetAttributes( Player_aPlayer[ m_nPlayerIndex ].m_ahTextBoxScopeInfo[i] );
				pTextArea->oColorBorder.fAlpha = 0.0f;

				m_afScopeInfoIntensity[i] = 0.0f;
			}
		}
	} else {
		// Targeting a bot...

		if( m_pScopeTargetedBot != (CBot *)pTargetedEntity ) {
			// Targeting a different bot than before...

			m_pScopeTargetedBot = (CBot *)pTargetedEntity;

			BotClass_e nBotClass = m_pScopeTargetedBot->m_pBotDef->m_nClass;

			switch( nBotClass ) {
			case BOTCLASS_GRUNT:
				m_pwszScopeTargetName = Game_apwszPhrases[ GAMEPHRASE_GRUNT ];
				break;

			case BOTCLASS_ELITE_GUARD:
				m_pwszScopeTargetName = Game_apwszPhrases[ GAMEPHRASE_GUARD ];
				break;

			case BOTCLASS_JUMP_TROOPER:
				m_pwszScopeTargetName = Game_apwszPhrases[ GAMEPHRASE_TROOPER ];
				break;

			case BOTCLASS_TITAN:
				m_pwszScopeTargetName = Game_apwszPhrases[ GAMEPHRASE_TITAN ];
				break;

			case BOTCLASS_SCOUT:
				m_pwszScopeTargetName = Game_apwszPhrases[ GAMEPHRASE_SNIPER ];
				break;

			case BOTCLASS_PREDATOR:
				m_pwszScopeTargetName = Game_apwszPhrases[ GAMEPHRASE_PREDATOR ];
				break;

			case BOTCLASS_SITEWEAPON:
				m_pwszScopeTargetName = Game_apwszPhrases[ GAMEPHRASE_SENTRY ];
				break;

			case BOTCLASS_MORTAR:
				m_pwszScopeTargetName = Game_apwszPhrases[ GAMEPHRASE_MORTAR ];
				break;

			case BOTCLASS_CORROSIVE:
				m_pwszScopeTargetName = Game_apwszPhrases[ GAMEPHRASE_CORROSIVE ];
				break;

			default:
				m_pwszScopeTargetName = NULL;
				break;
			}

			m_nScopeInfoDisplayCount = 1;
			m_fScopeInfoDisplaySecs = 0.5f;
			m_nScopeInfoBorderIndex = 0;
			m_fScopeInfoBorderSecs = 0.0f;

			for( i=0; i<RETICLE_SCOPE_INFO_SLOT_COUNT; ++i ) {
				m_afScopeInfoIntensity[i] = 1.0f;
			}
		}
	}
}


void CReticle::_InitScopeInfoTextBoxes( void ) {
	FTextArea_t *pTextArea;
	u32 i;

	for( i=0; i<RETICLE_SCOPE_INFO_SLOT_COUNT; ++i ) {
		pTextArea = ftext_GetAttributes( Player_aPlayer[ m_nPlayerIndex ].m_ahTextBoxScopeInfo[i] );

		pTextArea->bVisible = TRUE;
		pTextArea->fNumberOfLines = 1;
		pTextArea->fBorderThicknessX = 0.0025f;
		pTextArea->fBorderThicknessY = 0.0025f;
		pTextArea->oColorForeground.OpaqueWhite();
		pTextArea->oColorBackground.Set( 0.0f, 0.0f, 0.0f, 0.0f );
		pTextArea->oColorBorder.Set( 1.0f, 1.0f, 1.0f, 0.0f );
		pTextArea->ohFont = _SCOPE_ZOOM_FONT_CODE;
		pTextArea->oHorzAlign = FTEXT_HORZ_ALIGN_LEFT;

#if 0
pTextArea->fBorderThicknessX = 0.0025f;
pTextArea->fBorderThicknessY = 0.0025f;
pTextArea->oColorBorder.OpaqueWhite();
#endif

		pTextArea->fUpperLeftX = 0.5f - 0.5f*_SCOPE_ZOOM_TBOX_WIDTH;
		pTextArea->fLowerRightX = 0.5f + 0.5f*_SCOPE_ZOOM_TBOX_WIDTH;
		pTextArea->fUpperLeftY = 0.5f - 0.5f*_SCOPE_ZOOM_TBOX_HEIGHT;
		pTextArea->fLowerRightY = 0.5f + 0.5f*_SCOPE_ZOOM_TBOX_HEIGHT;
	}
}


void CReticle::SetType( Type_e nType ) {
	FTextArea_t *pTextArea;
	u32 i;
	CPlayer *pPlayer = &Player_aPlayer[ m_nPlayerIndex ];

	FASSERT( IsCreated() );
	FASSERT( m_nType>=0 && m_nType<TYPE_COUNT );
	FASSERT( nType >= 0 && nType < TYPE_COUNT );

	if( (m_nType != nType) && !(m_nFlags & FLAG_OVERRIDE_WEAPON_REQUESTS) ){
		if( nType == TYPE_DROID_SCOPE ) {
			// Switching to the scope...

			if( (CPlayer::m_nPlayerCount == 1) && Game_pFullscreenRenderTarget && (Game_pFcnFullscreenRenderTargetReadyCallback == NULL) ) {
				Game_pFcnFullscreenRenderTargetReadyCallback = _ScopeFullscreenRenderTargetReadyCallback;
				ftex_ActivateRenderTarget(Game_pFullscreenRenderTarget, TRUE);
			}

			m_fScopeHeading = 0.0f;
			m_fScopePitch = 0.0f;
			m_fScopeZoomMultiplier = 1.0f;
			m_pScopeTargetedBot = NULL;
			m_pwszScopeTargetName = NULL;
			m_fScopeInfoDisplaySecs = 0.0f;
			m_nScopeInfoDisplayCount = 0;
			m_nScopeInfoBorderIndex = 0;
			m_fScopeInfoBorderSecs = 0.0f;
			m_fScopeFlashMagSecs = 0.25f;
			m_nScopeFlashMagCount = 6;
			m_fScopeUnitJolt = 0.0f;
			FMATH_CLEARBITMASK( m_nFlags, FLAG_DISPLAY_SCOPE_TARGET_INFO );

			for( i=0; i<RETICLE_SCOPE_INFO_SLOT_COUNT; ++i ) {
				pTextArea = ftext_GetAttributes( pPlayer->m_ahTextBoxScopeInfo[i] );
				pTextArea->oColorBorder.fAlpha = 0.0f;

				m_afScopeInfoIntensity[i] = 0.0f;
			}

			ftext_GetAttributes( pPlayer->m_hTextBoxScopeZoom )->bVisible = TRUE;
			_InitScopeInfoTextBoxes();
		} else {
			if( m_nType == TYPE_DROID_SCOPE ) {
				// Switching away from the scope...

				if( Game_pFcnFullscreenRenderTargetReadyCallback == _ScopeFullscreenRenderTargetReadyCallback ) {
					Game_pFcnFullscreenRenderTargetReadyCallback = NULL;
					ftex_ActivateRenderTarget(Game_pFullscreenRenderTarget, FALSE);
				}

				ftext_GetAttributes( pPlayer->m_hTextBoxScopeZoom )->bVisible = FALSE;

				for( i=0; i<RETICLE_SCOPE_INFO_SLOT_COUNT; ++i ) {
					pTextArea = ftext_GetAttributes( pPlayer->m_ahTextBoxScopeInfo[i] );
					pTextArea->oColorBorder.fAlpha = 0.0f;

					ftext_GetAttributes( pPlayer->m_ahTextBoxScopeInfo[i] )->bVisible = FALSE;
				}
			}
		}

		TargetSys_Reset();
		m_nType = nType;
	}
}


void CReticle::_Draw_MirrorXY( Type_e nType ) {
	f32 fNormScale, fNormScaleY;
	s32 nTexIndex;

	nTexIndex = m_anTexIndex[nType];

	if( m_aTexInst[ nTexIndex ].GetTexDef() == NULL ) {
		return;
	}

	FViewport_t *pViewport = Player_aPlayer[m_nPlayerIndex].m_pViewportOrtho3D;
	fviewport_SetActive( pViewport );

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_SetTexture( &m_aTexInst[ nTexIndex ] );

	fNormScale = m_fNormScale * m_aTexInfo[ nTexIndex ].fScale;
	fNormScaleY = fNormScale * m_fUnitAnimScale;

	if( !(m_nFlags & FLAG_ACTIVE_COLOR) ) {
		// Inactive color...

		m_aFDrawVtx[0].ColorRGBA = m_InactiveColor;
		m_aFDrawVtx[1].ColorRGBA = m_InactiveColor;
		m_aFDrawVtx[2].ColorRGBA = m_InactiveColor;
		m_aFDrawVtx[3].ColorRGBA = m_InactiveColor;
		m_aFDrawVtx[4].ColorRGBA = m_InactiveColor;
		m_aFDrawVtx[5].ColorRGBA = m_InactiveColor;
		m_aFDrawVtx[6].ColorRGBA = m_InactiveColor;
		m_aFDrawVtx[7].ColorRGBA = m_InactiveColor;
	} else {
		// Active color...

		m_aFDrawVtx[0].ColorRGBA = m_ActiveColor;
		m_aFDrawVtx[1].ColorRGBA = m_ActiveColor;
		m_aFDrawVtx[2].ColorRGBA = m_ActiveColor;
		m_aFDrawVtx[3].ColorRGBA = m_ActiveColor;
		m_aFDrawVtx[4].ColorRGBA = m_ActiveColor;
		m_aFDrawVtx[5].ColorRGBA = m_ActiveColor;
		m_aFDrawVtx[6].ColorRGBA = m_ActiveColor;
		m_aFDrawVtx[7].ColorRGBA = m_ActiveColor;
	}
	fNormScale	*= 0.5f;
	fNormScaleY *= 0.5f;

	//upper left
	m_aFDrawVtx[0].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX - fNormScale );
	m_aFDrawVtx[0].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY + fNormScaleY);

	m_aFDrawVtx[1].Pos_MS.x = m_aFDrawVtx[0].Pos_MS.x;
	m_aFDrawVtx[1].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY);

	m_aFDrawVtx[2].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX);
	m_aFDrawVtx[2].Pos_MS.y = m_aFDrawVtx[0].Pos_MS.y;

	m_aFDrawVtx[3].Pos_MS.x = m_aFDrawVtx[2].Pos_MS.x;
	m_aFDrawVtx[3].Pos_MS.y = m_aFDrawVtx[1].Pos_MS.y;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, m_aFDrawVtx, 4 );

	// upper right
	m_aFDrawVtx[0].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX + fNormScale );
	m_aFDrawVtx[0].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY + fNormScaleY);

	m_aFDrawVtx[1].Pos_MS.x = m_aFDrawVtx[0].Pos_MS.x;
	m_aFDrawVtx[1].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY);

	m_aFDrawVtx[2].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX);
	m_aFDrawVtx[2].Pos_MS.y = m_aFDrawVtx[0].Pos_MS.y;
	
	m_aFDrawVtx[3].Pos_MS.x = m_aFDrawVtx[2].Pos_MS.x;
	m_aFDrawVtx[3].Pos_MS.y = m_aFDrawVtx[1].Pos_MS.y;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, m_aFDrawVtx, 4 );

	// bottom left
	m_aFDrawVtx[0].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX - fNormScale );
	m_aFDrawVtx[0].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY - fNormScaleY);

	m_aFDrawVtx[1].Pos_MS.x = m_aFDrawVtx[0].Pos_MS.x;
	m_aFDrawVtx[1].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY);

	m_aFDrawVtx[2].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX);
	m_aFDrawVtx[2].Pos_MS.y = m_aFDrawVtx[0].Pos_MS.y;
	
	m_aFDrawVtx[3].Pos_MS.x = m_aFDrawVtx[2].Pos_MS.x;
	m_aFDrawVtx[3].Pos_MS.y = m_aFDrawVtx[1].Pos_MS.y;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, m_aFDrawVtx, 4 );

	//bottom right
	m_aFDrawVtx[0].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX + fNormScale );
	m_aFDrawVtx[0].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY - fNormScaleY);

	m_aFDrawVtx[1].Pos_MS.x = m_aFDrawVtx[0].Pos_MS.x;
	m_aFDrawVtx[1].Pos_MS.y = pViewport->HalfRes.y * (m_fNormOriginY);

	m_aFDrawVtx[2].Pos_MS.x = pViewport->HalfRes.y * (m_fNormOriginX);
	m_aFDrawVtx[2].Pos_MS.y = m_aFDrawVtx[0].Pos_MS.y;
	
	m_aFDrawVtx[3].Pos_MS.x = m_aFDrawVtx[2].Pos_MS.x;
	m_aFDrawVtx[3].Pos_MS.y = m_aFDrawVtx[1].Pos_MS.y;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, m_aFDrawVtx, 4 );

	if( IsDrawEnabled() ) {
		if( m_fUnitAnimScale < 1.0f ) {
			m_fUnitAnimScale += _ANIMATE_SPEED * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fUnitAnimScale, 1.0f );
		}
	} else {
		if( m_fUnitAnimScale > 0.0f ) {
			m_fUnitAnimScale -= _ANIMATE_SPEED * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fUnitAnimScale, 0.0f );
		}
	}
}


void CReticle::_DisplayAmmoText( void ) {
	m_fAmmoTextFlashSecs -= FLoop_fPreviousLoopSecs;
	if( m_fAmmoTextFlashSecs <= 0.0f ) {
		switch( m_nAmmoTextFlashCount ) {
		case 0:
			m_nAmmoTextFlashCount = 2;
			m_fAmmoTextFlashSecs = _AMMO_TEXT_FLASH_ON_SECS;
			break;

		case 1:
			m_nAmmoTextFlashCount = 0;
			m_fAmmoTextFlashSecs = _AMMO_TEXT_FLASH_OFF_SECS;
			break;

		case 2:
			m_nAmmoTextFlashCount = 1;
			m_fAmmoTextFlashSecs = _AMMO_TEXT_FLASH_ON_SECS;
			break;

		default:
			FASSERT_NOW;
			break;
		};
	}

	if( m_nAmmoTextFlashCount == 0 ) {
		// Text flashing is off...
		return;
	}

	if( MultiplayerMgr.ExclusiveText( m_nPlayerIndex ) ) {
		// Multiplayer has text up, so don't draw
		return;
	}

	cwchar *pwszText;
	CPlayer *pPlayer = &Player_aPlayer[ m_nPlayerIndex ];

	switch( m_nAmmoText ) {
	case AMMO_TEXT_NO:
		pwszText = Game_apwszPhrases[ GAMEPHRASE_RETICLE_NO_AMMO ];
		break;

	case AMMO_TEXT_LOW:
		pwszText = Game_apwszPhrases[ GAMEPHRASE_RETICLE_LOW_AMMO ];
		break;

	case AMMO_TEXT_RELOAD:
		pwszText = Game_apwszPhrases[ GAMEPHRASE_RETICLE_RELOAD ];
		break;

	default:
		return;
	};

	FViewport_t *pViewport = Player_aPlayer[m_nPlayerIndex].m_pViewportOrtho3D;
	fviewport_SetActive( pViewport );
	f32 fNormScale = m_fNormScale * m_aTexInfo[ m_anTexIndex[m_nType] ].fScale * m_fUnitAnimScale;
	f32 fTextY, fUnitAlpha;

	FTextArea_t *pTextArea = ftext_GetAttributes( pPlayer->m_hTextBoxLowNoAmmo );

	// You might think you want pViewport->fAspectHOW here, but the text
	// system is hardwired for a 4:3 screen, so we hardwire to match.
	fTextY = 0.375f - 0.375f * (m_fNormOriginY - fNormScale - 0.06f);

	pTextArea->fUpperLeftY = fTextY - 0.5f*_AMMO_TEXT_TBOX_HEIGHT;
	pTextArea->fLowerRightY = fTextY + 0.5f*_AMMO_TEXT_TBOX_HEIGHT;

	fUnitAlpha = m_fAmmoTextFlashSecs * (1.0f / _AMMO_TEXT_FLASH_ON_SECS);
	FMATH_CLAMPMAX( fUnitAlpha, 1.0f );

	pTextArea->oColorForeground.Set( _AMMO_TEXT_RED, _AMMO_TEXT_GREEN, _AMMO_TEXT_BLUE, fUnitAlpha );

	ftext_PrintString( pPlayer->m_hTextBoxLowNoAmmo, pwszText );
}


