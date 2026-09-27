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
// Draw a generated Cross/Circle/Triangle/Square glyph (0..3) or the Options button (4) in the
// caller's current fdraw coordinate system.
extern void wpr_drawutils_DrawPlayStationGlyph( u32 nGlyph, f32 fCenterX, f32 fCenterY, f32 fRadius );
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
											f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes );
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
