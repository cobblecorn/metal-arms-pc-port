//////////////////////////////////////////////////////////////////////////////////////
// letterbox.cpp - Letterbox bars.
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
// 09/19/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "letterbox.h"
#include "fmath.h"
#include "fdraw.h"
#include "frenderer.h"
#include "floop.h"
#include "fviewport.h"
#include "Hud2.h"
#include "fvtxpool.h"
#include "PauseScreen.h"
#include "player.h"

#define _BAR_MAX_SIZE_Y				0.22f
#define _BAR_SLIDE_RATE				(1.0f / 0.22f)		// Y units per second
#define _SCREEN_LEFT_X				-0.5f
#define _SCREEN_RIGHT_X				0.5f
#define _SCREEN_TOP_Y				0.5f
#define _SCREEN_BOTTOM_Y			-0.5f
#define _SCREEN_Z					1.0f

#define _NUM_VERTS_NEEDED			12 // 0-5=top vertices, 6-11=bottom vertices

static BOOL _bSystemInitialized;

static f32 _fUnitSlideOn;
static LetterboxState_e _nState;

static BOOL _bHUDWSEnable = FALSE;
static BOOL _bHUDDrawEnable = FALSE;

BOOL letterbox_InitSystem( void ) {

	FASSERT( !_bSystemInitialized );

	_nState = LETTERBOX_STATE_MOTIONLESS;
	_fUnitSlideOn = 0.0f;

	_bSystemInitialized = TRUE;

	return TRUE;
}

void letterbox_UninitSystem( void ) {
	if( _bSystemInitialized ) {
		_bSystemInitialized = FALSE;
	}
}

void letterbox_Work( void ) {
	FASSERT( _bSystemInitialized );

	switch( _nState ) {
	case LETTERBOX_STATE_SLIDING_ON:
		_fUnitSlideOn += FLoop_fPreviousLoopSecs * _BAR_SLIDE_RATE;

		if( _fUnitSlideOn >= 1.0f ) {
			_fUnitSlideOn = 1.0f;
			_nState = LETTERBOX_STATE_MOTIONLESS;
		}

		break;

	case LETTERBOX_STATE_SLIDING_OFF:
		_fUnitSlideOn -= FLoop_fPreviousLoopSecs * _BAR_SLIDE_RATE;

		if( _fUnitSlideOn <= 0.0f ) {
			_fUnitSlideOn = 0.0f;
			_nState = LETTERBOX_STATE_MOTIONLESS;
			CPauseScreen::SetEnabled(TRUE);
			CHud2* pHud = CHud2::GetCurrentHud();
			if (pHud) {

				if( _bHUDWSEnable ) {
					pHud->SetWSEnable(TRUE);
				}

				if( _bHUDDrawEnable ) {
					pHud->SetDrawEnabled(TRUE);
				}
			}
		}

		break;
	}
}

// Draws to the active viewport which is expected to be an Ortho3D viewport.
// Assumes that the matrix stack contains the identity matrix.
void letterbox_Draw( void ) {
	FViewport_t *pCurrentViewport;
	f32 fBarSizeY;

	FASSERT( _bSystemInitialized );

	pCurrentViewport = fviewport_GetActive();
	if( pCurrentViewport == NULL ) {
		// No viewport to render into...
		return;
	}

	if( _fUnitSlideOn == 0.0f ) {
		// Bars are completely off the screen...
		return;
	}

	fBarSizeY = _fUnitSlideOn * _BAR_MAX_SIZE_Y;

	frenderer_Push( FRENDERER_DRAW, NULL );

	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_SetTexture( NULL );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_SRC );

	// grab some fdraw verts
	FDrawVtx_t *pVtx = fvtxpool_GetArray( _NUM_VERTS_NEEDED );
	if( !pVtx ) {
		// can't draw without fdraw verts
		return;
	}

	// Bottom bar...
	pVtx[0].Pos_MS.Set( _SCREEN_LEFT_X * pCurrentViewport->Res.x, 
						_SCREEN_BOTTOM_Y * pCurrentViewport->Res.y,
						_SCREEN_Z );
	pVtx[0].ColorRGBA.OpaqueBlack();

	pVtx[1].Pos_MS.Set( _SCREEN_LEFT_X * pCurrentViewport->Res.x,
						(_SCREEN_BOTTOM_Y + fBarSizeY) * pCurrentViewport->Res.y,
						_SCREEN_Z );
	pVtx[1].ColorRGBA.OpaqueBlack();

	pVtx[2].Pos_MS.Set( _SCREEN_RIGHT_X * pCurrentViewport->Res.x, 
						_SCREEN_BOTTOM_Y * pCurrentViewport->Res.y,
						_SCREEN_Z );
	pVtx[2].ColorRGBA.OpaqueBlack();

	pVtx[3].Pos_MS = pVtx[2].Pos_MS;
	pVtx[3].ColorRGBA.OpaqueBlack();

	pVtx[4].Pos_MS = pVtx[1].Pos_MS;
	pVtx[4].ColorRGBA.OpaqueBlack();
	
	pVtx[5].Pos_MS.Set( _SCREEN_RIGHT_X * pCurrentViewport->Res.x, 
						(_SCREEN_BOTTOM_Y + fBarSizeY) * pCurrentViewport->Res.y,
						_SCREEN_Z );
	pVtx[5].ColorRGBA.OpaqueBlack();

	// Top bar...
	pVtx[6].Pos_MS.Set( _SCREEN_LEFT_X * pCurrentViewport->Res.x, 
						(_SCREEN_TOP_Y - fBarSizeY) * pCurrentViewport->Res.y,
						_SCREEN_Z );
	pVtx[6].ColorRGBA.OpaqueBlack();

	pVtx[7].Pos_MS.Set( _SCREEN_LEFT_X * pCurrentViewport->Res.x,
						_SCREEN_TOP_Y * pCurrentViewport->Res.y,
						_SCREEN_Z );
	pVtx[7].ColorRGBA.OpaqueBlack();
	
	pVtx[8].Pos_MS.Set( _SCREEN_RIGHT_X * pCurrentViewport->Res.x,
						(_SCREEN_TOP_Y - fBarSizeY) * pCurrentViewport->Res.y,
						_SCREEN_Z );
	pVtx[8].ColorRGBA.OpaqueBlack();
	
	pVtx[9].Pos_MS = pVtx[8].Pos_MS;
	pVtx[9].ColorRGBA.OpaqueBlack();
	
	pVtx[10].Pos_MS = pVtx[7].Pos_MS;
	pVtx[10].ColorRGBA.OpaqueBlack();
	
	pVtx[11].Pos_MS.Set( _SCREEN_RIGHT_X * pCurrentViewport->Res.x, 
						 _SCREEN_TOP_Y * pCurrentViewport->Res.y, 
						 _SCREEN_Z );
	pVtx[11].ColorRGBA.OpaqueBlack();

	fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, pVtx, 12 );

	frenderer_Pop();

	// return the vtxs back to the pool
	fvtxpool_ReturnArray( pVtx );
}

void letterbox_SlideOn( BOOL bImmediate ) {
	FASSERT( _bSystemInitialized );
	_nState = LETTERBOX_STATE_SLIDING_ON;

	if( _fUnitSlideOn == 0.0f ) {
		// Only get the states of the HUD once we know we made it all the way back
		_bHUDWSEnable = CHud2::GetCurrentHud()->IsWSEnabled();
		_bHUDDrawEnable = CHud2::GetCurrentHud()->IsDrawEnabled();
		CHud2::GetCurrentHud()->SetDrawEnabled(FALSE);
		CHud2::GetCurrentHud()->SetWSEnable(FALSE);
	}

	CPauseScreen::SetEnabled(FALSE);
	CPlayer::AbortAllPlayerScopeZoomMode( TRUE );
	if( bImmediate ) {
		// We don't want the letterbox to slide on, we just want to be in letterbox mode IMMEDIATELY!
		_fUnitSlideOn = 1.0f;
	}

	for( s32 i = 0; i < CPlayer::m_nPlayerCount; ++i ) {
		fcamera_GetCameraByIndex( i )->StunCamera( -1.0f );
	}
}

void letterbox_SlideOff( BOOL bImmediate ) {
	FASSERT( _bSystemInitialized );
	_nState = LETTERBOX_STATE_SLIDING_OFF;

	if( bImmediate ) {
		// We don't want the letterbox to slide off, we just want to be out of letterbox mode IMMEDIATELY!
		_fUnitSlideOn = 0.0f;
	}
}

void letterbox_Reset( void ) {
	FASSERT( _bSystemInitialized );
	_nState = LETTERBOX_STATE_MOTIONLESS;
	_fUnitSlideOn = 0.0f;
	CPauseScreen::SetEnabled(TRUE);
	CHud2::GetCurrentHud()->SetWSEnable(TRUE);
	CHud2::GetCurrentHud()->SetDrawEnabled(TRUE);
}

// Returns 0 if completely off the screen, or 1 if completely on the screen.
f32 letterbox_GetUnitSlideOnAmount( void ) {
	FASSERT( _bSystemInitialized );
	return _fUnitSlideOn;
}

LetterboxState_e letterbox_GetState( void ) {
	FASSERT( _bSystemInitialized );
	return _nState;
}

