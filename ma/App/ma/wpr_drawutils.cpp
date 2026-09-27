//////////////////////////////////////////////////////////////////////////////////////
// wpr_drawutils.cpp - wrapper drawing utils
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
#include "fang.h"
#include "wpr_drawutils.h"
#include "ftext.h"
#include "fclib.h"
#if defined(MA_PC_INPUT)
#include "pc_input.h"
#endif

FDrawVtx_t Wpr_DrawUtils_aVtx[4];// used by all fdraw functions

#if defined(MA_PC_INPUT)
Wpr_DrawUtils_ButtonHit_t Wpr_DrawUtils_aButtonHits[WPR_DRAWUTILS_BUTTON_HITS];

void wpr_drawutils_ClearButtonHits( void ) {
	fang_MemZero( Wpr_DrawUtils_aButtonHits, sizeof( Wpr_DrawUtils_aButtonHits ) );
}
#endif



// assumes all draw modes are setup and either no texture or the desired texture has been set.
void wpr_drawutils_DrawThickLine( f32 fX1, f32 fY1, 
								 f32 fX2, f32 fY2,
								 f32 fPixelThickness,
								 CFColorRGBA *pRGBA, 
								 f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	CFVec3 Pt1, Pt2, Pt3, Pt4;
	CFVec2 Norm, Perp;
	Pt1.Set( fX1 * fHalfXRes, fY1 * fHalfYRes, 1.0f );		
	Pt2.Set( fX2 * fHalfXRes, fY2 * fHalfYRes, 1.0f );
	Norm.Set( Pt1.x - Pt2.x, Pt1.y - Pt2.y );
	f32 fLenSq = Norm.Mag2();
	if( fLenSq > 0.01f ) {
		Norm *= fmath_InvSqrt( fLenSq );
	} else {
		Norm.Set( 1.0f, 0.0f );
	}
	Perp.Set( Norm.y, -Norm.x );
	Perp *= fPixelThickness * 0.5f;

	Pt3 = Pt1;
	Pt3.x += Perp.x;
	Pt3.y += Perp.y;

	Pt4 = Pt2;
	Pt4.x += Perp.x;
	Pt4.y += Perp.y;

	Pt1.x -= Perp.x;
	Pt1.y -= Perp.y;

	Pt2.x -= Perp.x;
	Pt2.y -= Perp.y;

	fdraw_SolidQuad( &Pt1, &Pt3, &Pt4, &Pt2, pRGBA );
}

void wpr_drawutils_DrawLeftRightArrow( f32 fLeftX,
									  f32 fRightX,
									  f32 fY,
									  f32 fPixelHeight,
									  f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	
	f32 fYMultiplier = (fHalfXRes/fHalfYRes);
	CFVec3 Pt1, Pt2, Pt3;
	f32 fHalfHeight = fPixelHeight * 0.5f;
	u32 i;
	f32 fX = fLeftX;// draw the left side 1st
	f32 fWidthMultiplier = -1.0f;

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	for( i=0; i < 2; i++ ) {

		Pt1.Set( fX * fHalfXRes, fY * fHalfYRes, 1.0f );
		Pt1.y -= (fPixelHeight * fYMultiplier);

		Pt2 = Pt1;
		Pt2.x += fHalfHeight * fWidthMultiplier;
		Pt2.y += (fHalfHeight * fYMultiplier);
		
		Pt3 = Pt1;
		Pt3.y += (fPixelHeight * fYMultiplier);

		CFColorRGBA Color( 0.0f, 0.47f, 0.75f, 0.70f ); 
		fdraw_SolidTriangle( &Pt1, &Pt2, &Pt3, &Color );

		fX = fRightX;// draw the right side 2nd
		fWidthMultiplier = 1.0f;
	}
}

void wpr_drawutils_DrawUpDownArrow( f32 fX,
									f32 fTopY,
									f32 fLowerY,
									f32 fPixelHeight,
									f32 fPixelWidth,
									f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	
	f32 fYMultiplier = (fHalfXRes/fHalfYRes);
	CFVec3 Pt1, Pt2, Pt3;
	f32 fHalfHeight = fPixelHeight * 0.5f * fYMultiplier;
	f32 fHalfWidth = fPixelWidth * 0.5f;
	CFColorRGBA Color( 0.0f, 0.47f, 0.75f, 0.70f ); 
	
	// draw the upper arrow
	Pt1.Set( fX * fHalfXRes, fTopY * fHalfYRes, 1.0f );
	Pt1.x += fHalfWidth;

	Pt2 = Pt1;
	Pt2.x -= fPixelWidth;
	
	Pt3 = Pt1;
	Pt3.x -= fHalfWidth;
	Pt3.y += fHalfHeight;

	fdraw_SolidTriangle( &Pt1, &Pt2, &Pt3, &Color );

	// draw the lower arrow
	Pt1.Set( fX * fHalfXRes, fLowerY * fHalfYRes, 1.0f );
	Pt1.x += fHalfWidth;

	Pt2 = Pt1;
	Pt2.x -= fHalfWidth;
	Pt2.y -= fHalfHeight;

	Pt3 = Pt1;
	Pt3.x -= fPixelWidth;
	
	fdraw_SolidTriangle( &Pt1, &Pt2, &Pt3, &Color );
}

void wpr_drawutils_DrawTextureToScreen( BOOL bColor,
									   CFTexInst *pTexInst,
									   f32 fCenterX, f32 fCenterY,
									   f32 fHeight,
									   f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	CFVec2 Lower, Upper;
	f32 fHalfX, fHalfY;
	
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_SetTexture( pTexInst );
	if( bColor ) {
        fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	} else {
		fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECALTEX_AI_INTENSITY );
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_ALPHA_TIMES_SRC );		
	}

	fHalfX = 0.5f * (fHeight * fHalfYRes/fHalfXRes);
	fHalfY = 0.5f * fHeight;

	Lower.Set( (fCenterX - fHalfX) * fHalfXRes,
			   (fCenterY - fHalfY) * fHalfYRes );		
	Upper.Set( (fCenterX + fHalfX) * fHalfXRes,
			   (fCenterY + fHalfY) * fHalfYRes );
	
	Wpr_DrawUtils_aVtx[0].ST.Set( 0.0f, 1.0f );
	Wpr_DrawUtils_aVtx[1].ST.Set( 0.0f, 0.0f );
	Wpr_DrawUtils_aVtx[2].ST.Set( 1.0f, 1.0f );
	Wpr_DrawUtils_aVtx[3].ST.Set( 1.0f, 0.0f );

	CFColorRGBA Color;
	if( bColor ) {
		Color.Set( 1.0f, 1.0f, 1.0f, 1.0f );
	} else {
		Color.Set( 0.75f, 0.75f, 0.75f, 1.0f );//0.3f, 0.59f, 0.11f, 1.0f );
	}

	Wpr_DrawUtils_aVtx[0].Pos_MS.Set( Lower.x, Lower.y, 1.0f ); 
	Wpr_DrawUtils_aVtx[0].ColorRGBA = Color;
		
	Wpr_DrawUtils_aVtx[1].Pos_MS.Set( Lower.x, Upper.y, 1.0f );						
	Wpr_DrawUtils_aVtx[1].ColorRGBA = Color;

	Wpr_DrawUtils_aVtx[2].Pos_MS.Set( Upper.x, Lower.y, 1.0f );
	Wpr_DrawUtils_aVtx[2].ColorRGBA = Color;

	Wpr_DrawUtils_aVtx[3].Pos_MS.Set( Upper.x, Upper.y, 1.0f );
	Wpr_DrawUtils_aVtx[3].ColorRGBA = Color;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, Wpr_DrawUtils_aVtx, 4 );
}

#if defined(MA_PC_INPUT)
// Keyboard labels for wrapper-menu prompts (A, B, Y, X) on the Xbox UI map. Q selects the secondary
// gameplay list, so menus use Escape for Back and R for the left-face X button instead.
static cwchar *_apwszKeyCapLabels[] = { L"Space", L"Esc", L"E", L"R" };

// The prompt font's (~f1) line metrics per unit of font scale, in screen fractions of height: the line's
// height, and how far its top sits below the print position (as ftext_GetLastPrintBounds() reports
// them). Measured from each prompt, key cap and chart label printed; these starting values are the
// measured ones at 1280x960.
static f32 _fPromptLineHeightPerScale = 0.041f, _fPromptLineTopPerScale = 0.0115f;

static void _MeasurePromptFont( f32 fPrintY, f32 fScale, f32 fTop, f32 fBottom ) {
	if( fScale > 0.0f && fBottom > fTop ) {
		_fPromptLineHeightPerScale = (fBottom - fTop) / fScale;
		_fPromptLineTopPerScale = (fTop - fPrintY / 0.75f) / fScale;
	}
}

void wpr_drawutils_MeasureFontLine( f32 fPrintY, f32 fScale ) {
	f32 fLeft, fTop, fRight, fBottom;
	if( ftext_GetLastPrintBounds( &fLeft, &fTop, &fRight, &fBottom ) ) {
		_MeasurePromptFont( fPrintY, fScale, fTop, fBottom );
	}
}

// A keyboard key cap: the key's name on a raised key (see wpr_drawutils.h).
BOOL wpr_drawutils_DrawKeyCap( cwchar *pwszLabel, f32 fTextX, f32 fTextY, wchar cAlign, f32 fFontScale, f32 fMinWidth,
							   f32 fXScale, f32 fYScale, f32 *pfLeft, f32 *pfTop, f32 *pfRight, f32 *pfBottom ) {
	const f32 fPadX = 0.006f, fPadY = 0.003f;

	ftext_Printf( cAlign == L'C' ? fTextX : ( cAlign == L'R' ? fTextX - fPadX : fTextX + fPadX ), fTextY, L"~f1~C%ls~w0~a%lc~s%.2f%ls",
				  WprDataTypes_pwszSolidWhiteTextColor, cAlign, fFontScale, pwszLabel );
	f32 fLeft, fTop, fRight, fBottom;
	if( !ftext_GetLastPrintBounds( &fLeft, &fTop, &fRight, &fBottom ) ) {
		return FALSE;
	}
	_MeasurePromptFont( fTextY, fFontScale, fTop, fBottom );
	fLeft -= fPadX;
	fRight += fPadX;
	fTop -= fPadY;
	fBottom += fPadY;
	if( fRight - fLeft < fMinWidth ) {
		const f32 fGrow = fMinWidth - (fRight - fLeft);
		if( cAlign == L'C' ) {
			fLeft -= 0.5f * fGrow;
			fRight += 0.5f * fGrow;
		} else if( cAlign == L'R' ) {
			fLeft -= fGrow;
		} else {
			fRight += fGrow;
		}
	}

	// screen fractions -> the caller's space (y up, origin at the center)
	#define _CAP_X( f )		( ( (f) * 2.0f - 1.0f ) * fXScale )
	#define _CAP_Y( f )		( ( 1.0f - (f) * 2.0f ) * fYScale )
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_SetTexture( NULL );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	const f32 fDepth = 0.005f;	// the key's lower lip, in screen fractions of height
	CFVec3 a( _CAP_X( fLeft ), _CAP_Y( fTop ), 1.0f ), b( _CAP_X( fRight ), _CAP_Y( fTop ), 1.0f );
	CFVec3 c( _CAP_X( fRight ), _CAP_Y( fBottom + fDepth ), 1.0f ), d( _CAP_X( fLeft ), _CAP_Y( fBottom + fDepth ), 1.0f );
	CFColorRGBA Lip( 0.02f, 0.04f, 0.10f, 0.90f );
	fdraw_SolidQuad( &a, &b, &c, &d, &Lip );
	c.y = d.y = _CAP_Y( fBottom );
	CFColorRGBA Face( 0.10f, 0.16f, 0.30f, 0.90f );
	fdraw_SolidQuad( &a, &b, &c, &d, &Face );
	CFColorRGBA Edge( 0.55f, 0.72f, 0.95f, 1.0f );
	fdraw_SolidLine( &a, &b, &Edge ); fdraw_SolidLine( &b, &c, &Edge );
	fdraw_SolidLine( &c, &d, &Edge ); fdraw_SolidLine( &d, &a, &Edge );
	#undef _CAP_X
	#undef _CAP_Y

	if( pfLeft ) *pfLeft = fLeft;
	if( pfTop ) *pfTop = fTop;
	if( pfRight ) *pfRight = fRight;
	if( pfBottom ) *pfBottom = fBottom + fDepth;
	return TRUE;
}

BOOL wpr_drawutils_DrawKeyCapCentered( cwchar *pwszLabel, f32 fTextX, f32 fCenterY, wchar cAlign, f32 fFontScale, f32 fMinWidth,
									   f32 fXScale, f32 fYScale, f32 *pfLeft, f32 *pfTop, f32 *pfRight, f32 *pfBottom ) {
	const f32 fTextY = (fCenterY - (0.5f * _fPromptLineHeightPerScale + _fPromptLineTopPerScale) * fFontScale) * 0.75f;
	return wpr_drawutils_DrawKeyCap( pwszLabel, fTextX, fTextY, cAlign, fFontScale, fMinWidth, fXScale, fYScale,
									 pfLeft, pfTop, pfRight, pfBottom );
}

// A thick stroke from a to b, fWidth across.
static void _GlyphStroke( const CFVec3 &a, const CFVec3 &b, f32 fWidth, CFColorRGBA *pColor ) {
	f32 fDX = b.x - a.x, fDY = b.y - a.y;
	const f32 fLength = fmath_Sqrt( fDX * fDX + fDY * fDY );
	if( fLength <= 0.0f ) {
		return;
	}
	const f32 fNX = -fDY / fLength * 0.5f * fWidth, fNY = fDX / fLength * 0.5f * fWidth;
	// run past the ends by half the width so the corners of outlines close
	fDX *= 0.5f * fWidth / fLength; fDY *= 0.5f * fWidth / fLength;
	CFVec3 p( a.x - fDX + fNX, a.y - fDY + fNY, a.z ), q( b.x + fDX + fNX, b.y + fDY + fNY, b.z );
	CFVec3 r( b.x + fDX - fNX, b.y + fDY - fNY, b.z ), s( a.x - fDX - fNX, a.y - fDY - fNY, a.z );
	fdraw_SolidQuad( &p, &q, &r, &s, pColor );
}

// A closed polygon of nCorners corners on a circle of fRadius (the first corner at fStartAngle), drawn
// filled (fWidth <= 0) or as a thick outline.
static void _GlyphPolygon( f32 fX, f32 fY, f32 fRadius, u32 nCorners, f32 fStartAngle, f32 fWidth, CFColorRGBA *pColor ) {
	CFVec3 Center( fX, fY, 1.0f ), Prev, Next;
	for( u32 n=0; n <= nCorners; n++ ) {
		const f32 fAngle = fStartAngle + FMATH_2PI * (f32)n / (f32)nCorners;
		Next.Set( fX + fmath_Cos( fAngle ) * fRadius, fY + fmath_Sin( fAngle ) * fRadius, 1.0f );
		if( n ) {
			if( fWidth > 0.0f ) {
				_GlyphStroke( Prev, Next, fWidth, pColor );
			} else {
				fdraw_SolidQuad( &Center, &Prev, &Next, &Center, pColor );
			}
		}
		Prev = Next;
	}
}

void wpr_drawutils_DrawFaceButton( BOOL bPlayStation, u32 nFace, f32 fX, f32 fY, f32 fRadius, f32 fHalfXRes, f32 fHalfYRes ) {
	const f32 fCenterX = (fX * 2.0f - 1.0f) * fHalfXRes, fCenterY = (1.0f - fY * 2.0f) * fHalfYRes;
	const f32 fRadiusPx = fRadius * 2.0f * fHalfYRes;
	if( bPlayStation ) {
		wpr_drawutils_DrawPlayStationGlyph( nFace, fCenterX, fCenterY, fRadiusPx );
		return;
	}

	// Xbox: the button's color with its letter
	static cwchar *apwszLetters[4] = { L"A", L"B", L"Y", L"X" };
	CFColorRGBA Color, Rim( 0.02f, 0.03f, 0.06f, 0.95f );
	if( nFace == 0 ) Color.Set( 0.30f, 0.72f, 0.22f, 1.0f );
	else if( nFace == 1 ) Color.Set( 0.86f, 0.22f, 0.18f, 1.0f );
	else if( nFace == 2 ) Color.Set( 0.95f, 0.72f, 0.12f, 1.0f );
	else Color.Set( 0.18f, 0.45f, 0.92f, 1.0f );

	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_SetTexture( NULL );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	const FDrawCullDir_e nOldCull = fdraw_GetCullDir();
	fdraw_SetCullDir( FDRAW_CULLDIR_NONE );
	_GlyphPolygon( fCenterX, fCenterY, fRadiusPx, 24, 0.0f, 0.0f, &Color );
	_GlyphPolygon( fCenterX, fCenterY, fRadiusPx, 24, 0.0f, fRadiusPx * 0.12f, &Rim );
	fdraw_SetCullDir( nOldCull );

	if( nFace < 4 && _fPromptLineHeightPerScale > 0.0f ) {
		const f32 fScale = 1.5f * fRadius / _fPromptLineHeightPerScale;	// the letter a little taller than the radius
		ftext_Printf( fX, (fY - (0.5f * _fPromptLineHeightPerScale + _fPromptLineTopPerScale) * fScale) * 0.75f,
					  L"~f1~C%ls~w0~aC~s%.2f%ls", WprDataTypes_pwszSolidWhiteTextColor, fScale, apwszLetters[nFace] );
	}
}

void wpr_drawutils_DrawMouseGlyph( u32 nButton, f32 fX, f32 fY, f32 fHeight, f32 fHalfXRes, f32 fHalfYRes ) {
	const f32 fCenterX = (fX * 2.0f - 1.0f) * fHalfXRes, fCenterY = (1.0f - fY * 2.0f) * fHalfYRes;
	const f32 fB = fHeight * fHalfYRes;		// half height, pixels
	const f32 fA = fB * 0.62f;				// half width
	const f32 fSplitY = fCenterY + 0.30f * fB;	// the buttons are the part above this
	CFColorRGBA Body( 0.04f, 0.05f, 0.09f, 0.92f ), Rim( 0.55f, 0.72f, 0.95f, 1.0f ), Lit( 0.45f, 0.68f, 1.00f, 1.0f );

	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_SetTexture( NULL );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	const FDrawCullDir_e nOldCull = fdraw_GetCullDir();
	fdraw_SetCullDir( FDRAW_CULLDIR_NONE );

	// the body: an ellipse
	const u32 nCorners = 28;
	CFVec3 Center( fCenterX, fCenterY, 1.0f ), Prev, Next;
	for( u32 n=0; n <= nCorners; n++ ) {
		const f32 fAngle = FMATH_2PI * (f32)n / (f32)nCorners;
		Next.Set( fCenterX + fmath_Cos( fAngle ) * fA, fCenterY + fmath_Sin( fAngle ) * fB, 1.0f );
		if( n ) {
			fdraw_SolidQuad( &Center, &Prev, &Next, &Center, &Body );
		}
		Prev = Next;
	}

	// the lit button: the ellipse above the split, on its side of the middle
	if( nButton == 1 || nButton == 2 ) {
		const f32 fSide = (nButton == 1) ? -1.0f : 1.0f;
		const f32 fEndAngle = FMATH_PI - 0.30469265f;	// where the ellipse meets the split: sin = 0.30 (asin 0.30 = 0.3047)
		CFVec3 Corner( fCenterX, fSplitY, 1.0f );
		const u32 nSteps = 10;
		for( u32 n=0; n <= nSteps; n++ ) {
			// from the top (90 degrees) out to the split
			const f32 fAngle = FMATH_HALF_PI + (fEndAngle - FMATH_HALF_PI) * (f32)n / (f32)nSteps;
			Next.Set( fCenterX - fSide * fmath_Cos( fAngle ) * fA, fCenterY + fmath_Sin( fAngle ) * fB, 1.0f );
			if( n ) {
				fdraw_SolidQuad( &Corner, &Prev, &Next, &Corner, &Lit );
			}
			Prev = Next;
		}
	}

	// the outline, the split and the wheel
	const f32 fLine = FMATH_MAX( 1.5f, fB * 0.07f );
	for( u32 n=0; n <= nCorners; n++ ) {
		const f32 fAngle = FMATH_2PI * (f32)n / (f32)nCorners;
		Next.Set( fCenterX + fmath_Cos( fAngle ) * fA, fCenterY + fmath_Sin( fAngle ) * fB, 1.0f );
		if( n ) {
			_GlyphStroke( Prev, Next, fLine, &Rim );
		}
		Prev = Next;
	}
	const f32 fHalfSplit = fA * fmath_Sqrt( 1.0f - 0.30f * 0.30f );
	CFVec3 a( fCenterX - fHalfSplit, fSplitY, 1.0f ), b( fCenterX + fHalfSplit, fSplitY, 1.0f );
	_GlyphStroke( a, b, fLine, &Rim );
	a.Set( fCenterX, fSplitY, 1.0f ); b.Set( fCenterX, fCenterY + fB, 1.0f );
	_GlyphStroke( a, b, fLine, &Rim );
	a.Set( fCenterX, fSplitY + 0.22f * fB, 1.0f ); b.Set( fCenterX, fSplitY + 0.48f * fB, 1.0f );
	_GlyphStroke( a, b, fLine * 2.2f, &Rim );

	fdraw_SetCullDir( nOldCull );
}

// The retail data only contains Xbox button art. These generated glyphs keep the PlayStation
// presentation self-contained: A/B/Y/X map to Cross/Circle/Triangle/Square respectively.
void wpr_drawutils_DrawPlayStationGlyph( u32 i, f32 fX, f32 fY, f32 fRadius ) {
	CFColorRGBA Color;
	if( i == 0 ) Color.Set( 0.45f, 0.68f, 1.00f, 1.0f );		// Cross
	else if( i == 1 ) Color.Set( 1.00f, 0.38f, 0.38f, 1.0f );	// Circle
	else if( i == 2 ) Color.Set( 0.25f, 0.90f, 0.70f, 1.0f );	// Triangle
	else if( i == 3 ) Color.Set( 1.00f, 0.52f, 0.80f, 1.0f );	// Square
	else Color.Set( 0.88f, 0.88f, 0.95f, 1.0f );				// Options

	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_SetTexture( NULL );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

	const FDrawCullDir_e nOldCull = fdraw_GetCullDir();
	fdraw_SetCullDir( FDRAW_CULLDIR_NONE );	// the strokes and fans below wind both ways
	const f32 fWidth = fRadius * 0.16f;	// symbol stroke
	const f32 fSymbol = fRadius * 0.50f;	// symbol half size
	CFColorRGBA Body( 0.04f, 0.05f, 0.09f, 0.92f ), Rim( 0.55f, 0.58f, 0.66f, 1.0f );
	if( i > 3 ) {
		// Options: a small pill-shaped button with three lines on it
		const f32 fHalfWidth = fRadius * 0.95f, fHalfHeight = fRadius * 0.55f;
		CFVec3 a( fX - fHalfWidth, fY + fHalfHeight, 1.0f ), b( fX + fHalfWidth, fY + fHalfHeight, 1.0f );
		CFVec3 c( fX + fHalfWidth, fY - fHalfHeight, 1.0f ), d( fX - fHalfWidth, fY - fHalfHeight, 1.0f );
		fdraw_SolidQuad( &a, &b, &c, &d, &Body );
		const f32 fRim = fRadius * 0.08f;
		_GlyphStroke( a, b, fRim, &Rim ); _GlyphStroke( b, c, fRim, &Rim );
		_GlyphStroke( c, d, fRim, &Rim ); _GlyphStroke( d, a, fRim, &Rim );
		for( s32 n=-1; n <= 1; n++ ) {
			a.Set( fX - fHalfWidth * 0.5f, fY + n * fHalfHeight * 0.45f, 1.0f );
			b.Set( fX + fHalfWidth * 0.5f, fY + n * fHalfHeight * 0.45f, 1.0f );
			_GlyphStroke( a, b, fRadius * 0.10f, &Color );
		}
		fdraw_SetCullDir( nOldCull );
		return;
	}

	// the round button, then its symbol
	_GlyphPolygon( fX, fY, fRadius, 24, 0.0f, 0.0f, &Body );
	_GlyphPolygon( fX, fY, fRadius, 24, 0.0f, fRadius * 0.08f, &Rim );
	if( i == 0 ) {
		CFVec3 a( fX - fSymbol, fY - fSymbol, 1.0f ), b( fX + fSymbol, fY + fSymbol, 1.0f );
		_GlyphStroke( a, b, fWidth, &Color );
		a.Set( fX - fSymbol, fY + fSymbol, 1.0f ); b.Set( fX + fSymbol, fY - fSymbol, 1.0f );
		_GlyphStroke( a, b, fWidth, &Color );
	} else if( i == 1 ) {
		_GlyphPolygon( fX, fY, fSymbol, 20, 0.0f, fWidth, &Color );
	} else if( i == 2 ) {
		// the triangle's centroid a little below the center, so it looks centered
		_GlyphPolygon( fX, fY - fSymbol * 0.12f, fSymbol * 1.1f, 3, FMATH_HALF_PI, fWidth, &Color );
	} else {
		_GlyphPolygon( fX, fY, fSymbol * 1.1f, 4, FMATH_PI * 0.25f, fWidth, &Color );
	}
	fdraw_SetCullDir( nOldCull );
}
#endif

#if defined(MA_PC_INPUT)
#define _PROMPT_ICON_SIZE		0.80f	// the visible button's height, in prompt lines
#define _PROMPT_ICON_CENTER		0.50f	// the icon's center, in prompt lines below the line's top
#define _PROMPT_ART_FILL		0.70f	// how much of the pad art's height its button fills (the art has a margin)
#define _PROMPT_ICON_GAP		0.008f	// between the icon and its instruction, in screen fractions across

// PC prompts: each button's icon (the pad art, a generated PlayStation glyph, or a key cap) sits flush
// left of its instruction and is centered on the instruction's line, instead of the retail layout's
// icon floating above and to the left of the text. The retail positions still decide each prompt's row
// and where it starts; prompts sharing a row flow left to right without overlapping.
void wpr_drawutils_DrawButtonOverlay( Wpr_DataTypes_ScreenData_t *pScreen,
									  u32 nDrawButtonMask,
									 f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes,
									 u32 nControllerPort ) {
	const u32 nKeyCaps = sizeof( _apwszKeyCapLabels ) / sizeof( _apwszKeyCapLabels[0] );
	const f32 fAspect = fHalfYRes / fHalfXRes;	// fractions across per fraction down, for square shapes
	f32 fRowTextY = -1.0f, fRowNextLeft = 0.0f;	// where the next prompt on the current row may start

	for( u32 i=0; i < pScreen->nNumButtons; i++ ) {
		Wpr_DataTypes_ButtonLayout_t *pButton = &pScreen->paButtons[i];
		if( !(nDrawButtonMask & (1<<i)) ) {
			continue;
		}

		const f32 fUnitHeight = pButton->fPixelSize / (2.0f * fHalfYRes);
		const f32 fTextY = (((pButton->fBiPolarUnitY - 1.0f) * -0.5f) * 0.75f) - (fUnitHeight * 0.10f);	// ftext units (0..0.75 down)
		const f32 fScale = pButton->fFontScale;
		const f32 fLineHeight = _fPromptLineHeightPerScale * fScale;
		const f32 fLineTop = fTextY / 0.75f + _fPromptLineTopPerScale * fScale;
		const f32 fCenterY = fLineTop + _PROMPT_ICON_CENTER * fLineHeight;
		const f32 fIconSize = _PROMPT_ICON_SIZE * fLineHeight;

		// start where the retail icon started, or after the previous prompt on this row
		f32 fLeft = (pButton->fBiPolarUnitX - 0.5f * fUnitHeight * fAspect + 1.0f) * 0.5f;
		if( fRowTextY >= 0.0f && FMATH_FABS( fRowTextY - fTextY ) < 0.02f ) {
			fLeft = FMATH_MAX( fLeft, fRowNextLeft );
		}

		f32 fIconL = fLeft, fIconR = fLeft + fIconSize * fAspect;
		f32 fIconT = fCenterY - 0.5f * fIconSize, fIconB = fCenterY + 0.5f * fIconSize;
		if( i < nKeyCaps && pcinput_UseKeyboardPromptsForPort( nControllerPort ) ) {
			// a key cap naming the key; its label is centered on the line like the icons
			f32 fL, fT, fR, fB;
			if( wpr_drawutils_DrawKeyCapCentered( _apwszKeyCapLabels[i], fLeft, fCenterY, L'L', fScale * 0.72f, fIconSize * fAspect,
												  fHalfXRes, fHalfYRes, &fL, &fT, &fR, &fB ) ) {
				fIconL = fL; fIconT = fT; fIconR = fR; fIconB = fB;
			}
		} else if( i < nKeyCaps && pcinput_UsePlayStationPromptsForPort( nControllerPort ) ) {
			wpr_drawutils_DrawPlayStationGlyph( i, (fIconL + fIconR - 1.0f) * fHalfXRes, (1.0f - 2.0f * fCenterY) * fHalfYRes,
												0.5f * fIconSize * 2.0f * fHalfYRes );
		} else {
			wpr_drawutils_DrawTextureToScreen( TRUE,
				pButton->pTexture,
				fIconL + fIconR - 1.0f,
				1.0f - 2.0f * fCenterY,
				2.0f * fIconSize / _PROMPT_ART_FILL,
				fScaleMultiplier, fHalfXRes, fHalfYRes );
		}

		// the instruction
		const f32 fTextX = fIconR + _PROMPT_ICON_GAP;
		ftext_Printf( fTextX,
			fTextY,
			L"~f1~C%ls~w0~a%lc~s%.2f%ls",
			WprDataTypes_pwszButtonTextColor,
			L'L',
			fScale,
			pButton->pwszInstructions );
		f32 fTextL = fTextX, fTextT = fLineTop, fTextR = fTextX, fTextB = fLineTop + fLineHeight;
		if( ftext_GetLastPrintBounds( &fTextL, &fTextT, &fTextR, &fTextB ) ) {
			_MeasurePromptFont( fTextY, fScale, fTextT, fTextB );
			fRowTextY = fTextY;
			fRowNextLeft = fTextR + 0.02f;
		}

		if( i < WPR_DRAWUTILS_BUTTON_HITS ) {
			// clicking the icon or its instruction acts as that button
			Wpr_DrawUtils_ButtonHit_t *pHit = &Wpr_DrawUtils_aButtonHits[i];
			pHit->fLeft = FMATH_MIN( fIconL, fTextL );
			pHit->fTop = FMATH_MIN( fIconT, fTextT );
			pHit->fRight = FMATH_MAX( fIconR, fTextR );
			pHit->fBottom = FMATH_MAX( fIconB, fTextB );
			pHit->bDrawn = TRUE;
		}
	}
}
#else
void wpr_drawutils_DrawButtonOverlay( Wpr_DataTypes_ScreenData_t *pScreen,
									  u32 nDrawButtonMask,
									 f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes,
									 u32 nControllerPort ) {
	(void)nControllerPort;
	u32 i;
	Wpr_DataTypes_ButtonLayout_t *pButton;
	f32 fUnitHeight;

	for( i=0; i < pScreen->nNumButtons; i++ ) {
		pButton = &pScreen->paButtons[i];

		if( nDrawButtonMask & (1<<i) ) {
			fUnitHeight = pButton->fPixelSize / (2.0f * fHalfYRes);
			f32 fTextUnitX = ((pButton->fBiPolarUnitX + 1.0f) * 0.5f) + (fUnitHeight * 0.25f);
			const f32 fTextUnitY = (((pButton->fBiPolarUnitY - 1.0f) * -0.5f) * 0.75f) - (fUnitHeight * 0.10f);
			wpr_drawutils_DrawTextureToScreen( TRUE,
				pButton->pTexture,
				pButton->fBiPolarUnitX,
				pButton->fBiPolarUnitY,
				fUnitHeight,
				fScaleMultiplier, fHalfXRes, fHalfYRes );

			// draw the button instructions
			ftext_Printf( fTextUnitX,
				fTextUnitY,
				L"~f1~C%ls~w0~a%lc~s%.2f%ls",
				WprDataTypes_pwszButtonTextColor,
				L'L',
				pButton->fFontScale,
				pButton->pwszInstructions );
		}
	}
}
#endif

void wpr_drawutils_DrawPhrase( f32 fX, f32 fY,
							  cwchar *pwszColor,
							  cwchar *pwszAlignmentCode,
							  f32 fScale,					
							  cwchar *pwszText,
							  cwchar *pwszBlinkCode ) {
	
	if( !pwszBlinkCode ) {
        ftext_Printf( fX, fY,
					  L"~f1~C%ls~w0~a%ls~s%.2f%ls", 
					  pwszColor,
					  pwszAlignmentCode,
					  fScale, 
					  pwszText );
	} else {
		ftext_Printf( fX, fY,
					  L"%ls~f1~C%ls~w0~a%ls~s%.2f%ls", 
					  pwszBlinkCode,
					  pwszColor,
					  pwszAlignmentCode,
					  fScale, 
					  pwszText );
	}
}

void wpr_drawutils_DrawMesh_XlatOnly( Wpr_DataTypes_MeshLayout_t *pMesh, f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	if( pMesh->pMeshInst ) {
		wpr_drawutils_SetupMtx( pMesh->pMeshInst,
			pMesh->fScale * fScaleMultiplier,
			pMesh->fBiPolarUnitX * fHalfXRes,
			pMesh->fBiPolarUnitY * fHalfYRes,
			pMesh->fDrawZ );
		pMesh->pMeshInst->Draw( FVIEWPORT_PLANESMASK_ALL );
	}
}

void wpr_drawutils_DrawMesh_WithRot( Wpr_DataTypes_MeshLayout_t *pMesh,
									f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes, 
									f32 fRotX, f32 fRotY, f32 fRotZ ) {
	if( pMesh->pMeshInst ) {
		wpr_drawutils_SetupMtx( pMesh->pMeshInst,
			pMesh->fScale,// * fScaleMultiplier,
			pMesh->fBiPolarUnitX * fHalfXRes,
			pMesh->fBiPolarUnitY * fHalfYRes,
			pMesh->fDrawZ,
			fRotX, fRotY, fRotZ );
		pMesh->pMeshInst->Draw( FVIEWPORT_PLANESMASK_ALL );
	}
}

void wpr_drawutils_SetupMtx( CFMeshInst *pMeshInst,
							f32 fScale,
							f32 fX, f32 fY, f32 fZ, 
							f32 fRotX/*=0.0f*/, f32 fRotY/*=0.0f*/, f32 fRotZ/*=0.0f*/ ) {
	if( pMeshInst ) {
		// build an Mtx
		CFMtx43A::m_Temp.SetRotationYXZ( fRotY, fRotX, fRotZ );
		
		CFMtx43A::m_Temp.m_vPos.Set( fX, fY, fZ );

		pMeshInst->m_Xfm.BuildFromMtx( CFMtx43A::m_Temp, fScale );
	}
}

#if defined(MA_PC_INPUT)
Wpr_DrawUtils_Box_t Wpr_DrawUtils_aLastArrows[2];
Wpr_DrawUtils_TickBar_t Wpr_DrawUtils_LastTickBar;

// ortho pixels (y up, origin at the center) -> screen fractions
static void _RecordBox( Wpr_DrawUtils_Box_t *pBox, const CFVec2 &Lower, const CFVec2 &Upper, f32 fHalfXRes, f32 fHalfYRes ) {
	pBox->fLeft = (Lower.x / fHalfXRes + 1.0f) * 0.5f;
	pBox->fRight = (Upper.x / fHalfXRes + 1.0f) * 0.5f;
	pBox->fTop = (1.0f - Upper.y / fHalfYRes) * 0.5f;
	pBox->fBottom = (1.0f - Lower.y / fHalfYRes) * 0.5f;
}
#endif

void wpr_drawutils_DrawSelectionArrows( Wpr_DataTypes_TextLayout_t *pText,
									   CFTexInst *pTexInst,
									   f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {

	CFVec2 Lower, Upper;
	f32 fX, fY, fLen, fHeight, fWidth;

	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_SetTexture( pTexInst );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

	fLen = (f32)fclib_wcslen( pText->pwszText );
	fLen *= pText->fTickSpaceFactor;
	fLen *= pText->fScale;
	fLen *= (1.0f/640.0f);
	fLen *= fHalfXRes * 2.0f;

	fX = (pText->fUnitX - 0.5f) * 2.0f * fHalfXRes;
	fY = (0.5f - (pText->fUnitY*(640.0f/480.0f))) * 2.0f * fHalfYRes;

	// fonts are drawn with the cursor at the top corner, move that to the bottom corner
	fHeight = (0.1f * fHalfYRes * (fHalfXRes/fHalfYRes));
	fY -= (fHeight * 0.75f);
	fWidth = (0.05f * fHalfXRes);

	switch( pText->nAlignment ) {
	case WPR_DATATYPES_ALIGN_LEFT:
		fX -= fWidth;
		break;
	case WPR_DATATYPES_ALIGN_RIGHT:
		fX += fLen;
		break;
	case WPR_DATATYPES_ALIGN_CENTERED:
		fX -= fLen * 0.5f;
		break;
	}
	Lower.Set( fX, fY );
	Upper.Set( Lower.x + fWidth,
			   Lower.y + fHeight );
#if defined(MA_PC_INPUT)
	_RecordBox( &Wpr_DrawUtils_aLastArrows[0], Lower, Upper, fHalfXRes, fHalfYRes );
#endif

	// draw the left arrow
	Wpr_DrawUtils_aVtx[0].Pos_MS.Set( Lower.x, Lower.y, 1.0f ); 
	Wpr_DrawUtils_aVtx[0].ColorRGBA.Set( 0.0f, 0.47f, 0.75f, 0.90f );//0.80f, 0.80f, 0.80f, 0.90f );
	Wpr_DrawUtils_aVtx[0].ST.Set( 1.0f, 0.0f );

	Wpr_DrawUtils_aVtx[1].Pos_MS.Set( Lower.x, Upper.y, 1.0f );						
	Wpr_DrawUtils_aVtx[1].ColorRGBA = Wpr_DrawUtils_aVtx[0].ColorRGBA;
	Wpr_DrawUtils_aVtx[1].ST.Set( 1.0f, 1.0f );

	Wpr_DrawUtils_aVtx[2].Pos_MS.Set( Upper.x, Lower.y, 1.0f );
	Wpr_DrawUtils_aVtx[2].ColorRGBA = Wpr_DrawUtils_aVtx[0].ColorRGBA;
	Wpr_DrawUtils_aVtx[2].ST.Set( 0.0f, 0.0f );

	Wpr_DrawUtils_aVtx[3].Pos_MS.Set( Upper.x, Upper.y, 1.0f );
	Wpr_DrawUtils_aVtx[3].ColorRGBA = Wpr_DrawUtils_aVtx[0].ColorRGBA;
	Wpr_DrawUtils_aVtx[3].ST.Set( 0.0f, 1.0f );

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, Wpr_DrawUtils_aVtx, 4 );

	// draw the right arrow
	Lower.x += fLen - fWidth;
	Upper.x += fLen - fWidth;
#if defined(MA_PC_INPUT)
	_RecordBox( &Wpr_DrawUtils_aLastArrows[1], Lower, Upper, fHalfXRes, fHalfYRes );
#endif

	Wpr_DrawUtils_aVtx[0].Pos_MS.Set( Lower.x, Lower.y, 1.0f ); 
	Wpr_DrawUtils_aVtx[0].ST.Set( 0.0f, 0.0f );

	Wpr_DrawUtils_aVtx[1].Pos_MS.Set( Lower.x, Upper.y, 1.0f );						
	Wpr_DrawUtils_aVtx[1].ST.Set( 0.0f, 1.0f );

	Wpr_DrawUtils_aVtx[2].Pos_MS.Set( Upper.x, Lower.y, 1.0f );
	Wpr_DrawUtils_aVtx[2].ST.Set( 1.0f, 0.0f );

	Wpr_DrawUtils_aVtx[3].Pos_MS.Set( Upper.x, Upper.y, 1.0f );
	Wpr_DrawUtils_aVtx[3].ST.Set( 1.0f, 1.0f );

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, Wpr_DrawUtils_aVtx, 4 );
}

void wpr_drawutils_DrawTickMarks( u32 nNumTicks, u32 nMaxTicks,
								 CFTexInst *pTexInst,
								 f32 fLowerX, f32 fLowerY,
								 f32 fHeight,
								 f32 fSpaceBetweenTicks, 
								 f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	u32 i;
	CFVec2 Lower, Upper;
	f32 fDeltaX = fSpaceBetweenTicks * fHalfXRes;

#if defined(MA_PC_INPUT)
	Wpr_DrawUtils_LastTickBar.fLeft = (fLowerX + 1.0f) * 0.5f;
	Wpr_DrawUtils_LastTickBar.fTop = (1.0f - (fLowerY + fHeight)) * 0.5f;
	Wpr_DrawUtils_LastTickBar.fBottom = (1.0f - fLowerY) * 0.5f;
	Wpr_DrawUtils_LastTickBar.fStep = fSpaceBetweenTicks * 0.5f;
	Wpr_DrawUtils_LastTickBar.fTickWidth = fHeight * 0.25f * 0.5f;
	Wpr_DrawUtils_LastTickBar.nTicks = nNumTicks;
	Wpr_DrawUtils_LastTickBar.nMaxTicks = nMaxTicks;
#endif

	if( nNumTicks == 0 ) {
		return;
	}
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_SetTexture( pTexInst );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

	Lower.Set( fLowerX * fHalfXRes, fLowerY * fHalfYRes );
	Upper.Set( Lower.x + (fHeight * 0.25f * fHalfXRes),
			   Lower.y + (fHeight * fHalfYRes ) );

	Wpr_DrawUtils_aVtx[0].ST.Set( 0.0f, 0.0f );
	Wpr_DrawUtils_aVtx[1].ST.Set( 0.0f, 1.0f );
	Wpr_DrawUtils_aVtx[2].ST.Set( 1.0f, 0.0f );
	Wpr_DrawUtils_aVtx[3].ST.Set( 1.0f, 1.0f );

	CFVec3 Color0, Color1, NewColor;
	Color0.Set( 0.45f, 0.55f, 1.0f );
	Color1.Set( 0.0f, 0.10f, 1.0f );

	f32 fPercentPerTick = (1.0f/(f32)nMaxTicks);
    
	for( i=0; i < nNumTicks; i++ ) {
		NewColor.ReceiveLerpOf( (f32)i * fPercentPerTick, Color0, Color1 );

		Wpr_DrawUtils_aVtx[0].Pos_MS.Set( Lower.x, Lower.y, 1.0f ); 
		Wpr_DrawUtils_aVtx[0].ColorRGBA.Set( NewColor.x, NewColor.y, NewColor.z, 1.0f );
			
		Wpr_DrawUtils_aVtx[1].Pos_MS.Set( Lower.x, Upper.y, 1.0f );						
		Wpr_DrawUtils_aVtx[1].ColorRGBA = Wpr_DrawUtils_aVtx[0].ColorRGBA;
	
		Wpr_DrawUtils_aVtx[2].Pos_MS.Set( Upper.x, Lower.y, 1.0f );
		Wpr_DrawUtils_aVtx[2].ColorRGBA = Wpr_DrawUtils_aVtx[0].ColorRGBA;
	
		Wpr_DrawUtils_aVtx[3].Pos_MS.Set( Upper.x, Upper.y, 1.0f );
		Wpr_DrawUtils_aVtx[3].ColorRGBA = Wpr_DrawUtils_aVtx[0].ColorRGBA;
	
		fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, Wpr_DrawUtils_aVtx, 4 );

		// prepare for the next iteration
		Lower.x += fDeltaX;
		Upper.x += fDeltaX;		
	}
}

void wpr_drawutils_ConvertTextCoordsToOrthoCoords( f32 fTextX, f32 fTextY,
												  f32 &rfOrthoX, f32 &rfOrthoY,
												  f32 fHalfXRes, f32 fHalfYRes ) {
	rfOrthoX = (fTextX - 0.5f) * 2.0f;
	rfOrthoY = ( (fTextY * (fHalfXRes/fHalfYRes)) * -2.0f ) + 1.0f;
}

void wpr_drawutils_ConvertOrthoCoordsToTextCoords( f32 fOrthoX, f32 fOrthoY,
												  f32 &rfTextX, f32 &rfTextY,												  
												  f32 fHalfXRes, f32 fHalfYRes ) {
	rfTextX = (fOrthoX * 0.5f) + 0.5f;
	rfTextY = ( (fOrthoY - 1.0f) * (-1.0f/2.0f) ) * (fHalfYRes/fHalfXRes);
}

