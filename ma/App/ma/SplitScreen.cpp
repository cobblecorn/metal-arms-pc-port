//////////////////////////////////////////////////////////////////////////////////////
// SplitScreen.cpp - Screen handler for split-screen multiplayer.
//
// Author: Dan Stanfill, Pinniped Software     
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
// 02/27/03 Stanfill    Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "SplitScreen.h"
#include "player.h"
#include "fcamera.h"
#include "gamecam.h"



///////////////////////////////////////////////////////////////////////////////
// Constants and Macros
///////////////////////////////////////////////////////////////////////////////

// For debugging huds etc. Draws a line around the NTSC safe title area.
//#define _SHOW_SAFE_AREA 1

// Half-width of border between viewports. Can be zero or greater integer
#define _VP_SPACING 1

///////////////////////////////////////////////////////////////////////////////
// Static Variables
///////////////////////////////////////////////////////////////////////////////

// Color of the spacing between viewports
static const CFColorRGBA _GapColor(0.0f, 0.0f, 0.5f, 1.0f);
static SplitType_e _eSplitType = SPLIT_HORIZONTAL;

///////////////////////////////////////////////////////////////////////////////
// Static function prototypes
///////////////////////////////////////////////////////////////////////////////
// The gameloop function to draw the safe area crashes on the gamecube, so we
// implement our own
static void _DrawSafeArea(void);

///////////////////////////////////////////////////////////////////////////////
// Inline functions
///////////////////////////////////////////////////////////////////////////////
FINLINE void _DrawQuad(f32 fLeft, f32 fRight, f32 fTop, f32 fBottom) {
	CFVec3 v0(fLeft,  fTop, 1.0f);
	CFVec3 v1(fRight, fTop, 1.0f);
	CFVec3 v2(fRight, fBottom, 1.0f);
	CFVec3 v3(fLeft,  fBottom, 1.0f);
	fdraw_SolidQuad(&v0, &v1, &v2, &v3, &_GapColor);
}

//-----------------------------------------------------------------------------
// Set the split priority to use. Must be called before SetupViewports.
extern void splitscreen_SetSplitType(SplitType_e eType)
{
	_eSplitType = eType;
}

//-----------------------------------------------------------------------------
// DrawBorders assumes the draw renderer has been set up and the camera 
// transform stack reinitialized to identity.
void splitscreen_DrawBorders(void)
{
#if (_VP_SPACING > 0)
	if (CPlayer::m_nPlayerCount > 1) {
		// Set a full-screen ortho viewport with pixel addressing
		FViewport_t *pPreviousVP = fviewport_GetActive();
		fviewport_SetActive(FViewport_pDefaultOrtho);

		fdraw_Depth_EnableWriting(FALSE);
		fdraw_Depth_SetTest(FDRAW_DEPTHTEST_ALWAYS);
		fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_SRC);
		fdraw_Color_SetFunc(FDRAW_COLORFUNC_DECAL_AI);

		CFVec3 aVerts[4];
		f32 fLeft, fRight, fTop, fBottom;

		if (_eSplitType == SPLIT_VERTICAL) {
			// Always draw the vertical line
			fLeft   = FViewport_pDefaultOrtho->HalfRes.x - (f32)_VP_SPACING;
			fRight  = FViewport_pDefaultOrtho->HalfRes.x + (f32)_VP_SPACING;
			fTop    = 0.0f;
			fBottom = FViewport_pDefaultOrtho->Res.y;
			_DrawQuad(fLeft, fRight, fTop, fBottom);

			// Draw a horizontal line
			if (CPlayer::m_nPlayerCount > 2) {
				fLeft = (CPlayer::m_nPlayerCount == 4) ? 0.0f : fRight;
				fRight = FViewport_pDefaultOrtho->Res.x;
				fTop = FViewport_pDefaultOrtho->HalfRes.y - (f32)_VP_SPACING;
				fBottom = FViewport_pDefaultOrtho->HalfRes.y + (f32)_VP_SPACING;
				_DrawQuad(fLeft, fRight, fTop, fBottom);
			}
		}
		else {
			// Always draw the horizontal line
			fLeft = 0.0f;
			fRight = FViewport_pDefaultOrtho->Res.x;
			fTop = FViewport_pDefaultOrtho->HalfRes.y - (f32)_VP_SPACING;
			fBottom = FViewport_pDefaultOrtho->HalfRes.y + (f32)_VP_SPACING;
			_DrawQuad(fLeft, fRight, fTop, fBottom);

			// Draw a vertical line
			if (CPlayer::m_nPlayerCount > 2) {
				fLeft = FViewport_pDefaultOrtho->HalfRes.x - (f32)_VP_SPACING;
				fRight = FViewport_pDefaultOrtho->HalfRes.x + (f32)_VP_SPACING;
				fTop = (CPlayer::m_nPlayerCount == 4) ? 0.0f : fBottom;
				fBottom = FViewport_pDefaultOrtho->Res.y;
				_DrawQuad(fLeft, fRight, fTop, fBottom);
			}
		}

		fviewport_SetActive(pPreviousVP);
	}
#endif

#ifdef _SHOW_SAFE_AREA
	_DrawSafeArea();
#endif
}

//-----------------------------------------------------------------------------
BOOL splitscreen_SetupViewports( int nPlayerNum, int nTotalPlayers ) {

	CFCamera* cam;			//CPS 4.7.03

	// Determine our viewport bounds
	u32 nLeft = 0;
	u32 nTop = 0;
	u32 nWidth = FVid_Mode.nPixelsAcross;
	u32 nHeight = FVid_Mode.nPixelsDown;

	// Tweak the ortho viewport boundary for safe area. That way the hud can
	// just fill the viewport up and not worry about the edges.
	s32 nTweakH = (s32)(0.075f * FVid_Mode.nPixelsAcross);
	s32 nTweakV = (s32)(0.075f * FVid_Mode.nPixelsDown);
	u32 nSafeLeft = nTweakH;
	u32 nSafeTop = nTweakV;
	u32 nSafeWidth = nWidth - (nTweakH << 1);
	u32 nSafeHeight = nHeight - (nTweakV << 1);

	if (nTotalPlayers > 1) {

		switch (nPlayerNum) {
			case 3:
				nWidth = (nWidth >> 1) - _VP_SPACING;
				nHeight = (nHeight >> 1) - _VP_SPACING;
				nLeft = nSafeLeft = FVid_Mode.nPixelsAcross - nWidth;
				nTop  = nSafeTop  = FVid_Mode.nPixelsDown - nHeight;
				nSafeWidth = nWidth - nTweakH;
				nSafeHeight = nHeight - nTweakV;
				break;
			case 2:
				nWidth = (nWidth >> 1) - _VP_SPACING;
				nHeight = (nHeight >> 1) - _VP_SPACING;
				nTop = nSafeTop = FVid_Mode.nPixelsDown - nHeight;
				if (nTotalPlayers == 3)
					nLeft = nSafeLeft = FVid_Mode.nPixelsAcross - nWidth;
				else
					nSafeLeft = nTweakH;
				nSafeWidth = nWidth - nTweakH;
				nSafeHeight = nHeight - nTweakV;
				break;
			case 1:
				if (_eSplitType == SPLIT_HORIZONTAL) {
					nHeight = (nHeight >> 1) - _VP_SPACING;
					nSafeHeight = nHeight - nTweakV;
					switch (nTotalPlayers) {
						case 2:
							nTop = nSafeTop = FVid_Mode.nPixelsDown - nHeight;
							break;
						case 3:
							nWidth = (nWidth >> 1) - _VP_SPACING;
							nSafeWidth = nWidth - nTweakH;
							nTop = nSafeTop = FVid_Mode.nPixelsDown - nHeight;
							break;
						case 4:
							nWidth = (nWidth >> 1) - _VP_SPACING;
							nSafeWidth = nWidth - nTweakH;
							nLeft = nSafeLeft = FVid_Mode.nPixelsAcross - nWidth;
					}
				}
				else {
					nWidth = (nWidth >> 1) - _VP_SPACING;
					nSafeWidth = nWidth - nTweakH;
					nLeft = nSafeLeft = FVid_Mode.nPixelsAcross - nWidth;
					if (nTotalPlayers > 2) {
						nHeight = (nHeight >> 1) - _VP_SPACING;
						nSafeHeight = nHeight - nTweakV;
					}
				}
				break;
			case 0:
			default:
				if (_eSplitType == SPLIT_HORIZONTAL) {
					nHeight = (nHeight >> 1) - _VP_SPACING;
					nSafeHeight = nHeight - nTweakV;
					if (nTotalPlayers > 3) {
						nWidth = (nWidth >> 1) - _VP_SPACING;
						nSafeWidth = nWidth - nTweakH;
					}
				}
				else {
					nWidth = (nWidth >> 1) - _VP_SPACING;
					nSafeWidth = nWidth - nTweakH;
					if (nTotalPlayers > 3) {
						nHeight = (nHeight >> 1) - _VP_SPACING;
						nSafeHeight = nHeight - nTweakV;
					}
				}
				break;
		}
	}

	// Set up our ortho viewport
	Player_aPlayer[nPlayerNum].m_pViewportOrtho3D = fviewport_Create();
	if( Player_aPlayer[nPlayerNum].m_pViewportOrtho3D == NULL ) {
		DEVPRINTF( "game::_SetupViewports(): Unable to create Ortho3D viewport.\n" );
		goto _ExitVPWithError;
	}
	fviewport_InitOrtho3D( Player_aPlayer[nPlayerNum].m_pViewportOrtho3D,
						   0.1f, 1000.0f, nLeft, nTop, nWidth, nHeight);

	// Set up an ortho viewport encompassing only the safe area
	Player_aPlayer[nPlayerNum].m_pViewportSafeOrtho3D = fviewport_Create();
	if( Player_aPlayer[nPlayerNum].m_pViewportSafeOrtho3D == NULL ) {
		DEVPRINTF( "game::_SetupViewports(): Unable to create Safe Ortho3D viewport.\n" );
		goto _ExitVPWithError;
	}
	fviewport_InitOrtho3D( Player_aPlayer[nPlayerNum].m_pViewportSafeOrtho3D,
		0.1f, 1000.0f, nSafeLeft, nSafeTop, nSafeWidth, nSafeHeight);

	// Now get the viewport for this player's camera and fix it up with our
	// new viewport boundaries.
//CPS 4.7.03	CFCamera* cam = fcamera_GetCameraByIndex( PLAYER_CAM(nPlayerNum) );
	cam = fcamera_GetCameraByIndex( PLAYER_CAM(nPlayerNum) );			//CPS 4.7.03
	cam->MoveViewport(nLeft, nTop, nWidth, nHeight, _eSplitType == SPLIT_VERTICAL);

	return TRUE;

_ExitVPWithError:
	return FALSE;
}

//-----------------------------------------------------------------------------
// For a given player's screen area, calculates a new horizontal FOV which
// preserves the current vertical FOV. Note: This fov is actually half of
// the field of view.
f32 splitscreen_RemapFOV( s32 nPlayerIndex, f32 fFOV ) {
	// If we split vertically first, never remap the fov
	if (_eSplitType == SPLIT_VERTICAL)
		return fFOV;

	// If there are no players, one player or four players, no remap needed
	if ((CPlayer::m_nPlayerCount <= 1) || (CPlayer::m_nPlayerCount > 3))
		return fFOV;

	// Player 2 never needs a remap
	if (nPlayerIndex == 2)
		return fFOV;

	// We are two or 3 player. If we are 3 player, player one does not need a remap
	if ((nPlayerIndex == 1) && (CPlayer::m_nPlayerCount == 3))
		return fFOV;

	// OK, we need to remap the fov.
	f32 fVidHt = (f32)FVid_Mode.nPixelsDown;
	f32 fSin, fCos;
	fmath_SinCos( fFOV, &fSin, &fCos );
	return fmath_Atan( 2.0f * fVidHt * fSin , (fVidHt - (2.0f * _VP_SPACING)) * fCos );
}

//-----------------------------------------------------------------------------
// Just like RemapFOV, but in reverse.
f32 splitscreen_UnmapFOV( s32 nPlayerIndex, f32 fFOV )
{
	// If we split vertically first, never remap the fov
	if (_eSplitType == SPLIT_VERTICAL)
		return fFOV;

	// If there are no players, one player or four players, no remap needed
	if ((CPlayer::m_nPlayerCount <= 1) || (CPlayer::m_nPlayerCount > 3))
		return fFOV;

	// Player 2 never needs a remap
	if (nPlayerIndex == 2)
		return fFOV;

	// We are two or 3 player. If we are 3 player, player one does not need a remap
	if ((nPlayerIndex == 1) && (CPlayer::m_nPlayerCount == 3))
		return fFOV;

	// OK, we need to remap the fov.
	f32 fVidHt = (f32)FVid_Mode.nPixelsDown;
	f32 fSin, fCos;
	fmath_SinCos( fFOV, &fSin, &fCos );
	return fmath_Atan( (fVidHt - (2.0f * _VP_SPACING)) * fSin , 2.0f * fVidHt * fCos );
}

//-----------------------------------------------------------------------------
#ifdef _SHOW_SAFE_AREA
static void _DrawSafeArea(void) {
	FViewport_t* pVP = FViewport_pDefaultOrtho;
	fviewport_SetActive(pVP);
	fdraw_Depth_EnableWriting(FALSE);
	fdraw_Depth_SetTest(FDRAW_DEPTHTEST_ALWAYS);
	fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_SRC);
	fdraw_Color_SetFunc(FDRAW_COLORFUNC_DECAL_AI);

	f32 fLeft   = pVP->ScreenCorners.UpperLeft.x + 0.075f * pVP->Res.x;
	f32 fRight  = pVP->ScreenCorners.LowerRight.x - 0.075f * pVP->Res.x;
	f32 fTop    = pVP->ScreenCorners.UpperLeft.y + 0.075f * pVP->Res.y;
	f32 fBottom = pVP->ScreenCorners.LowerRight.y - 0.075f * pVP->Res.y;

	_DrawQuad(fLeft - 2.0f, fLeft, fTop, fBottom);
	_DrawQuad(fLeft, fRight, fTop - 2.0f, fTop);
	_DrawQuad(fRight, fRight + 2.0f, fTop, fBottom);
	_DrawQuad(fLeft, fRight, fBottom, fBottom + 2.0f);
}
#endif	// _SHOW_SAFE_AREA

