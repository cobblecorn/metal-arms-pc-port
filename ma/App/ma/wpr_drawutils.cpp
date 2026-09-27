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

void wpr_drawutils_DrawButtonOverlay( Wpr_DataTypes_ScreenData_t *pScreen, 
									  u32 nDrawButtonMask,
									 f32 fScaleMultiplier, f32 fHalfXRes, f32 fHalfYRes ) {
	u32 i;
	Wpr_DataTypes_ButtonLayout_t *pButton;
	f32 fUnitHeight;
	
	for( i=0; i < pScreen->nNumButtons; i++ ) {
		pButton = &pScreen->paButtons[i];

		if( nDrawButtonMask & (1<<i) ) {
			fUnitHeight = pButton->fPixelSize / (2.0f * fHalfYRes);

			wpr_drawutils_DrawTextureToScreen( TRUE, 
				pButton->pTexture,
				pButton->fBiPolarUnitX,
				pButton->fBiPolarUnitY,
				fUnitHeight,
				fScaleMultiplier, fHalfXRes, fHalfYRes );	
			
			// draw the button instructions
			ftext_Printf( ((pButton->fBiPolarUnitX + 1.0f) * 0.5f) + (fUnitHeight * 0.25f),
				(((pButton->fBiPolarUnitY - 1.0f) * -0.5f) * 0.75f) - (fUnitHeight * 0.10f),
				L"~f1~C%ls~w0~a%lc~s%.2f%ls", 
				WprDataTypes_pwszButtonTextColor,
				L'L',
				pButton->fFontScale,
				pButton->pwszInstructions );		

#if defined(MA_PC_INPUT)
			if( i < WPR_DRAWUTILS_BUTTON_HITS ) {
				// the icon (as wpr_drawutils_DrawTextureToScreen() sizes it) plus the instruction text
				Wpr_DrawUtils_ButtonHit_t *pHit = &Wpr_DrawUtils_aButtonHits[i];
				f32 fHalfX = 0.5f * (fUnitHeight * fHalfYRes/fHalfXRes), fHalfY = 0.5f * fUnitHeight;
				pHit->fLeft = (pButton->fBiPolarUnitX - fHalfX + 1.0f) * 0.5f;
				pHit->fRight = (pButton->fBiPolarUnitX + fHalfX + 1.0f) * 0.5f;
				pHit->fTop = (1.0f - (pButton->fBiPolarUnitY + fHalfY)) * 0.5f;
				pHit->fBottom = (1.0f - (pButton->fBiPolarUnitY - fHalfY)) * 0.5f;
				f32 fLeft, fTop, fRight, fBottom;
				if( ftext_GetLastPrintBounds( &fLeft, &fTop, &fRight, &fBottom ) ) {
					pHit->fLeft = FMATH_MIN( pHit->fLeft, fLeft );
					pHit->fTop = FMATH_MIN( pHit->fTop, fTop );
					pHit->fRight = FMATH_MAX( pHit->fRight, fRight );
					pHit->fBottom = FMATH_MAX( pHit->fBottom, fBottom );
				}
				pHit->bDrawn = TRUE;
			}
#endif
		}
	}
}

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

