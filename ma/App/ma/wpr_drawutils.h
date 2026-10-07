//////////////////////////////////////////////////////////////////////////////////////
// wpr_drawutils.h - wrapper drawing utils
//
// Author: Michael Starich   
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
// 11/05/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _WPR_DRAW_UTILS_H_
#define _WPR_DRAW_UTILS_H_ 1

#include "fang.h"
#include "fdraw.h"
#include "wpr_datatypes.h"


extern FDrawVtx_t Wpr_DrawUtils_aVtx[4];// used by all fdraw functions

#if defined(MA_PC_INPUT)
// Where wpr_drawutils_DrawButtonOverlay() last drew each prompt (A, B, Y, X), as fractions of the
// screen (0..1 across and down), so a mouse click on a prompt can act as that button.
#define WPR_DRAWUTILS_BUTTON_HITS	4
typedef struct {
	BOOL bDrawn;
	f32 fLeft, fTop, fRight, fBottom;
} Wpr_DrawUtils_ButtonHit_t;
extern Wpr_DrawUtils_ButtonHit_t Wpr_DrawUtils_aButtonHits[WPR_DRAWUTILS_BUTTON_HITS];
extern void wpr_drawutils_ClearButtonHits( void );
// Where wpr_drawutils_DrawSelectionArrows() last drew its left and right arrows, and where
// wpr_drawutils_DrawTickMarks() last drew its bar (drawn or not, the bar's full extent), in screen
// fractions, so the menus can make them clickable.
typedef struct {
	f32 fLeft, fTop, fRight, fBottom;
} Wpr_DrawUtils_Box_t;
extern Wpr_DrawUtils_Box_t Wpr_DrawUtils_aLastArrows[2];
typedef struct {
	f32 fLeft, fTop, fBottom;	// the first tick's left edge; the bar's top and bottom
	f32 fStep, fTickWidth;		// from one tick to the next; one tick's width
	u32 nTicks, nMaxTicks;		// the value shown, and the most it can be
} Wpr_DrawUtils_TickBar_t;
extern Wpr_DrawUtils_TickBar_t Wpr_DrawUtils_LastTickBar;
// Draw a generated Cross/Circle/Triangle/Square glyph (0..3) or the Options button (4) in the
// caller's current fdraw coordinate system: a dark round button of fRadius with the symbol on it.
// bYDown supports the screen-pixel coordinates used by message boxes.
extern void wpr_drawutils_DrawPlayStationGlyph( u32 nGlyph, f32 fCenterX, f32 fCenterY, f32 fRadius, BOOL bYDown = FALSE );
// Draw a keyboard key cap: pwszLabel printed at (fTextX, fTextY) (screen fractions, the text's top;
// cAlign L'L' puts the key's left there, L'C' its center, L'R' its right) on a raised key sized to the text and at
// least fMinWidth wide. The key is drawn in the caller's current fdraw space, where screen fraction f
// maps to ((f*2-1)*fXScale, (1-f*2)*fYScale). Returns the key's bounds in screen fractions.
// bScreenPixels selects top-left y-down pixels for message dialogs; default is centered y-up.
extern BOOL wpr_drawutils_DrawKeyCap( cwchar *pwszLabel, f32 fTextX, f32 fTextY, wchar cAlign, f32 fFontScale, f32 fMinWidth,
									  f32 fXScale, f32 fYScale, f32 *pfLeft, f32 *pfTop, f32 *pfRight, f32 *pfBottom, BOOL bScreenPixels = FALSE );
// A pad face button, nFace 0..3 = the bottom, right, top and left buttons (Xbox A, B, Y, X; PlayStation
// Cross, Circle, Triangle, Square), centered at (fX, fY) in screen fractions, fRadius a fraction of the
// screen's height. Drawn in the wrapper's ortho space (pixels, origin at the center, y up).
extern void wpr_drawutils_DrawFaceButton( BOOL bPlayStation, u32 nFace, f32 fX, f32 fY, f32 fRadius, f32 fHalfXRes, f32 fHalfYRes );
// A mouse fHeight tall (screen fraction) centered at (fX, fY): nButton 1 or 2 lights its left or right
// button; 0 draws it plain (moving the mouse). Same space as above.
extern void wpr_drawutils_DrawMouseGlyph( u32 nButton, f32 fX, f32 fY, f32 fHeight, f32 fHalfXRes, f32 fHalfYRes );
// After an ftext_Printf() in the prompt font (~f1) at ftext y fPrintY and scale fScale: measure its line,
// which the centered drawing here uses.
extern void wpr_drawutils_MeasureFontLine( f32 fPrintY, f32 fScale );
// The same, with the label's line centered on fCenterY (a screen fraction down) using the prompt font's
// measured line metrics.
extern BOOL wpr_drawutils_DrawKeyCapCentered( cwchar *pwszLabel, f32 fTextX, f32 fCenterY, wchar cAlign, f32 fFontScale, f32 fMinWidth,
											  f32 fXScale, f32 fYScale, f32 *pfLeft, f32 *pfTop, f32 *pfRight, f32 *pfBottom, BOOL bScreenPixels = FALSE );
// Plain text in the prompt font (pwszStyle: color codes etc. before it), its line centered on fCenterY like a key cap.
extern void wpr_drawutils_PrintPromptCentered( cwchar *pwszText, f32 fTextX, f32 fCenterY, wchar cAlign, f32 fFontScale, cwchar *pwszColor );
#endif


extern void wpr_drawutils_DrawThickLine( f32 fX1, f32 fY1,
										f32 fX2, f32 fY2,
										f32 fPixelThickness,
										CFColorRGBA *pRGBA, 
										f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
extern void wpr_drawutils_DrawLeftRightArrow( f32 fLeftX,
											 f32 fRightX,
											 f32 fY,
											 f32 fPixelHeight,
											 f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
extern void wpr_drawutils_DrawUpDownArrow( f32 fX,
										f32 fTopY,
										f32 fLowerY,
										f32 fPixelHeight,
										f32 fPixelWidth,
										f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
extern void wpr_drawutils_DrawTextureToScreen( BOOL bColor,
											  CFTexInst *pTexInst,
											  f32 fCenterX, f32 fCenterY,
											  f32 fHeight,
											  f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
extern void wpr_drawutils_DrawButtonOverlay( Wpr_DataTypes_ScreenData_t *pScreen,
										  u32 nDrawButtonMask,
										 f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes,
										 u32 nControllerPort );
extern void wpr_drawutils_DrawPhrase( f32 fX, f32 fY,
									 cwchar *pszColor,
									 cwchar *pszAlignmentCode,
									 f32 fScale,
									 cwchar *pszText,
									 cwchar *pszBlinkCode );
extern void wpr_drawutils_DrawMesh_XlatOnly( Wpr_DataTypes_MeshLayout_t *pMesh,
											f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
extern void wpr_drawutils_DrawMesh_WithRot( Wpr_DataTypes_MeshLayout_t *pMesh,
										   f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes,
										   f32 fRotX, f32 fRotY, f32 fRotZ );
extern void wpr_drawutils_SetupMtx( CFMeshInst *pMeshInst, 
								   f32 fScale,
								   f32 fX, f32 fY, f32 fZ,
								   f32 fRotX=0.0f, f32 fRotY=0.0f, f32 fRotZ=0.0f );
extern void wpr_drawutils_DrawSelectionArrows( Wpr_DataTypes_TextLayout_t *pText,
											  CFTexInst *pTexInst,
											  f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
extern void wpr_drawutils_DrawTickMarks( u32 nNumTicks, u32 nMaxTicks,
										CFTexInst *pTexInst,
										f32 fLowerX, f32 fLowerY,
										f32 fHeight, f32 fSpaceBetweenTicks, 
										f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
extern void wpr_drawutils_ConvertTextCoordsToOrthoCoords( f32 fTextX, f32 fTextY,
														  f32 &rfOrthoX, f32 &rfOrthoY,
														  f32 fHalfXRes, f32 fHalfYRes );
extern void wpr_drawutils_ConvertOrthoCoordsToTextCoords( f32 fOrthoX, f32 fOrthoY,
														 f32 &rfTextX, f32 &rfTextY,												  
														 f32 fHalfXRes, f32 fHalfYRes );



#endif
