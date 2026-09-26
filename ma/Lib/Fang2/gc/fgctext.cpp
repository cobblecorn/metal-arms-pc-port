//////////////////////////////////////////////////////////////////////////////////////
// fgctext.cpp
//
// Author: John Lafleur
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
// 02/09/03 JLafleur    Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fgc.h"
#include "ftext.h"
#include "fdraw.h"
#include "ftex.h"
#include "fgctex.h"
#include "fvtxpool.h"
#include "fgcxfm.h"
#include "frenderer.h"

extern FText_FTextPrintfLetter_t *FText_paoPrintfLetters;

extern void ftext_CalculateLetterCoordinates( FText_FTextPrintfLetter_t *pLetterStruct, 
										 f32 *pfUpperLeftX, f32 *pfUpperRightX, f32 *pfLowerLeftX, f32 *pfLowerRightX,
										 f32 *pfUpperY, f32 *pfLowerY );


//
//
//
void ftext_DrawTexturePageCharacters( FDataFntFile_Font_t *pFont, u32 nTexturePageIdx ) 
{

	f32 fZ = 1.0f;

	fdraw_SetTexture( &pFont->paoTexInsts[ nTexturePageIdx ] );

	// Calculate half texel offsets for better displaying of 1 to 1 mapped textures
	FTexDef_t *pTexDef = pFont->paoTexInsts[ nTexturePageIdx ].GetTexDef();
	f32 fTexOffsetX = 1.0f / ( pTexDef->TexInfo.nTexelsAcross * 2.0f );
	f32 fTexOffsetY = 1.0f / ( pTexDef->TexInfo.nTexelsDown * 2.0f );

	u32 nCharactersToRender = pFont->pauTextLengthPerTexturePages[ nTexturePageIdx ];
	//grab a pool of vertices large enough to render all these characters

	FDrawVtx_t *pVertices = NULL;
	u32 nRequestSize = nCharactersToRender * 6;
	do 
	{
		pVertices = fvtxpool_GetArray( nRequestSize, TRUE );
		if( !pVertices ) 
		{
			//allocation failed.  Half our request
			nRequestSize = ( ( nCharactersToRender >> 2 ) + ( nCharactersToRender % 2 ) ) * 6;
		}

	} while( (pVertices == NULL) && ( nRequestSize > 0 ) );

	if( nRequestSize == 0 ) 
	{
		DEVPRINTF( "[ FTEXT ] Could not allocate any vertices from the fvtxpool to render the characters -- aborting text rendering!\n" );
		return;
	}

	u32 nCharactersRendered = 0;
	u32 nCharactersPrepped = 0;
	u32 nLetterArrayIndex = 0;
	while( nCharactersRendered < nCharactersToRender ) 
	{
		u32 nPoolVerticesUsed = 0;
		while( ( nPoolVerticesUsed < nRequestSize ) && ( nCharactersPrepped < nCharactersToRender ) ) 
		{
			FASSERT ( nLetterArrayIndex < Fang_ConfigDefs.nText_MaxCharsPerFrame );

			FDataFntFile_Letter_t *pFontLetter = &pFont->paoFntLetters[ FText_paoPrintfLetters[ nLetterArrayIndex ].uFntLetterIdx ];
			if( ( FText_paoPrintfLetters[ nLetterArrayIndex ].hFont == pFont->oHandle ) && ( pFontLetter->uTexurePage == nTexturePageIdx ) ) 
			{
			
				FText_FTextPrintfLetter_t *pLetterStruct = &FText_paoPrintfLetters[ nLetterArrayIndex ];
				//calculate the character coordinates
				f32 fLetterUpperLeftX, fLetterUpperRightX, fLetterLowerLeftX, fLetterLowerRightX;
				f32 fLetterUpperY, fLetterLowerY; //the y values will be the same between left and right
				ftext_CalculateLetterCoordinates( pLetterStruct, 
					&fLetterUpperLeftX, &fLetterUpperRightX, &fLetterLowerLeftX, &fLetterLowerRightX,
					&fLetterUpperY, &fLetterLowerY );

				// Next, if the letter is going to be one to one aligned or debug printed, increment
				// the texel coordinates by half a texel... Otherwise forget it.
				f32 fThisTexOffsetX = 0.0f;
				f32 fThisTexOffsetY = 0.0f;
				if( ( pLetterStruct->nStats & _FTEXT_PRINTF_LETTERSTATS_DEBUG ) || 
					( pLetterStruct->nStats & _FTEXT_PRINTF_LETTERSTATS_OTOTEXELPIXELALIGNMENT ) )
				{
					fThisTexOffsetX = fTexOffsetX;
					fThisTexOffsetY = fTexOffsetY;
				}
				
				//now, fill the vertices...
				
				//first, precalculate the color
				CFColorRGBA oTempColor( FText_paoPrintfLetters[ nLetterArrayIndex ].oColor );

				// Upper Left
				pVertices[ nPoolVerticesUsed + 0 ].Pos_MS.Set( fLetterUpperLeftX, fLetterUpperY, fZ );
				pVertices[ nPoolVerticesUsed + 0 ].ColorRGBA = oTempColor;
				pVertices[ nPoolVerticesUsed + 0 ].ST.Set( pFontLetter->fUVUpperLeftX+fThisTexOffsetX, pFontLetter->fUVUpperLeftY + fThisTexOffsetY );

				// Upper Right
				pVertices[ nPoolVerticesUsed + 1 ].Pos_MS.Set( fLetterUpperRightX, fLetterUpperY, fZ );
				pVertices[ nPoolVerticesUsed + 1 ].ColorRGBA = oTempColor;
				pVertices[ nPoolVerticesUsed + 1 ].ST.Set( pFontLetter->fUVLowerRightX+fThisTexOffsetX, pFontLetter->fUVUpperLeftY + fThisTexOffsetY );

				// Lower Left
				pVertices[ nPoolVerticesUsed + 2 ].Pos_MS.Set( fLetterLowerLeftX, fLetterLowerY, fZ );
				pVertices[ nPoolVerticesUsed + 2 ].ColorRGBA = oTempColor;
				pVertices[ nPoolVerticesUsed + 2 ].ST.Set( pFontLetter->fUVUpperLeftX+fThisTexOffsetX, pFontLetter->fUVLowerRightY + fThisTexOffsetY );

				// Upper Right
				pVertices[ nPoolVerticesUsed + 3 ] = pVertices[ nPoolVerticesUsed + 1 ];

				// Lower Left
				pVertices[ nPoolVerticesUsed + 4 ] = pVertices[ nPoolVerticesUsed + 2 ];

				// Lower Right
				pVertices[ nPoolVerticesUsed + 5 ].Pos_MS.Set( fLetterLowerRightX, fLetterLowerY, fZ );
				pVertices[ nPoolVerticesUsed + 5 ].ColorRGBA = oTempColor;
				pVertices[ nPoolVerticesUsed + 5 ].ST.Set( pFontLetter->fUVLowerRightX+fThisTexOffsetX, pFontLetter->fUVLowerRightY + fThisTexOffsetY);

				//update the necessary counters
				nPoolVerticesUsed += 6;
				nCharactersPrepped++;
			}			
			nLetterArrayIndex++;
		}
		
		//draw the vertices
		fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, pVertices, nPoolVerticesUsed );
		nCharactersRendered = nCharactersPrepped;
	}

	fvtxpool_ReturnArray( pVertices );

#if _VERTS_USAGE_STATS
	//// Usage statistics.
	//
//	_uStatVertsUsedCurrentDraw += ( 6 * _poTempFont->aoViewportSegments[ ( _uIndex * Fang_ConfigDefs.nText_MaxViewportChangesPerDraw ) + _uCount ].uLength );
	//
	////
#endif

}
