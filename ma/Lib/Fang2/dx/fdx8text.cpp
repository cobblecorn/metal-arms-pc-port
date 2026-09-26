//////////////////////////////////////////////////////////////////////////////////////
// fdx8text.cpp
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
// 02/03/03 JLafleur    Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fdx8.h"
#include "ftext.h"
#include "fdraw.h"
#include "ftex.h"
#include "fdx8tex.h"
#include "fvtxpool.h"
#include "fdx8xfm.h"
#include "frenderer.h"
#include "fdx8sh.h"


extern FText_FTextPrintfLetter_t *FText_paoPrintfLetters;

extern void ftext_CalculateLetterCoordinates( FText_FTextPrintfLetter_t *pLetterStruct, 
										 f32 *pfUpperLeftX, f32 *pfUpperRightX, f32 *pfLowerLeftX, f32 *pfLowerRightX,
										 f32 *pfUpperY, f32 *pfLowerY );

#if FANG_PLATFORM_WIN
void ftext_DrawTexturePageCharacters( FDataFntFile_Font_t *pFont, u32 nTexturePageIdx ) 
{

	f32 fZ = 1.0f;

	fdraw_SetTexture( &pFont->paoTexInsts[ nTexturePageIdx ] );

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

				//calculate the character coordinates
				f32 fLetterUpperLeftX, fLetterUpperRightX, fLetterLowerLeftX, fLetterLowerRightX;
				f32 fLetterUpperY, fLetterLowerY; //the y values will be the same between left and right
				ftext_CalculateLetterCoordinates( &FText_paoPrintfLetters[ nLetterArrayIndex ], 
					&fLetterUpperLeftX, &fLetterUpperRightX, &fLetterLowerLeftX, &fLetterLowerRightX,
					&fLetterUpperY, &fLetterLowerY );

				//now, fill the vertices...
				
				//first, precalculate the color
				CFColorRGBA oTempColor( FText_paoPrintfLetters[ nLetterArrayIndex ].oColor );

				// Upper Left
				pVertices[ nPoolVerticesUsed + 0 ].Pos_MS.Set( fLetterUpperLeftX, fLetterUpperY, fZ );
				pVertices[ nPoolVerticesUsed + 0 ].ColorRGBA = oTempColor;
				pVertices[ nPoolVerticesUsed + 0 ].ST.Set( pFontLetter->fUVUpperLeftX, pFontLetter->fUVUpperLeftY );

				// Upper Right
				pVertices[ nPoolVerticesUsed + 1 ].Pos_MS.Set( fLetterUpperRightX, fLetterUpperY, fZ );
				pVertices[ nPoolVerticesUsed + 1 ].ColorRGBA = oTempColor;
				pVertices[ nPoolVerticesUsed + 1 ].ST.Set( pFontLetter->fUVLowerRightX, pFontLetter->fUVUpperLeftY );

				// Lower Left
				pVertices[ nPoolVerticesUsed + 2 ].Pos_MS.Set( fLetterLowerLeftX, fLetterLowerY, fZ );
				pVertices[ nPoolVerticesUsed + 2 ].ColorRGBA = oTempColor;
				pVertices[ nPoolVerticesUsed + 2 ].ST.Set( pFontLetter->fUVUpperLeftX, pFontLetter->fUVLowerRightY );

				// Upper Right
				pVertices[ nPoolVerticesUsed + 3 ] = pVertices[ nPoolVerticesUsed + 1 ];

				// Lower Left
				pVertices[ nPoolVerticesUsed + 4 ] = pVertices[ nPoolVerticesUsed + 2 ];

				// Lower Right
				pVertices[ nPoolVerticesUsed + 5 ].Pos_MS.Set( fLetterLowerRightX, fLetterLowerY, fZ );
				pVertices[ nPoolVerticesUsed + 5 ].ColorRGBA = oTempColor;
				pVertices[ nPoolVerticesUsed + 5 ].ST.Set( pFontLetter->fUVLowerRightX, pFontLetter->fUVLowerRightY );

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
#else
//
//
//
void ftext_DrawTexturePageCharacters( FDataFntFile_Font_t *pFont, u32 nTexturePageIdx ) 
{
	u32 nCharactersToRender = pFont->pauTextLengthPerTexturePages[ nTexturePageIdx ];

	fdraw_SetTexture( &pFont->paoTexInsts[ nTexturePageIdx ] );

	// Set fill mode...
	fdx8_SetRenderState_FILLMODE( FRenderer_nD3DFillMode );

	fdx8xfm_SetDXMatrices( FALSE );

	u32 nCharactersRendered = 0;
	u32 nCharactersPrepped = 0;
	u32 nLetterArrayIndex = 0;

	#define __DWORDS_PER_VERTEX			6
	#define __MAX_VERTICES_PER_ARRAY	340   // We can submit a max of 2047 DWORDS of data at a time

	u16 nVtxCount = nCharactersToRender * 4;

	u32 uPushSizeInDWORDs = (__DWORDS_PER_VERTEX * nVtxCount);

	u16 uArrayCount, uCurrentArrayVertCount = __MAX_VERTICES_PER_ARRAY;
	if ( nVtxCount <= __MAX_VERTICES_PER_ARRAY )
	{
		uArrayCount = 1;
	}
	else
	{
		uArrayCount = (nVtxCount + __MAX_VERTICES_PER_ARRAY) / __MAX_VERTICES_PER_ARRAY;
	}

	FASSERT( uArrayCount > 0 );

	FDX8_SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) );
//	fdx8sh_SetVertexType( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) );

	// Gain direct access to the pushbuffer. Note that we have 
	// to reserve overhead for the encoding parameters.
	DWORD *pPush;
	if ( FDX8_pDev->BeginPush( uPushSizeInDWORDs + 4 + uArrayCount, &pPush ) != S_OK )
	{
		return;
	}

	// Push the macro that starts things off
	*pPush++ = D3DPUSH_ENCODE( D3DPUSH_SET_BEGIN_END, 1 );
	
	// Specify the primitive type of the vertices that follow
	*pPush++ = D3DPT_QUADLIST;

	u32 nD3DColor;
	while( nCharactersPrepped < nCharactersToRender ) 
	{ 
		FASSERT ( nLetterArrayIndex < Fang_ConfigDefs.nText_MaxCharsPerFrame );

		FDataFntFile_Letter_t *pFontLetter = &pFont->paoFntLetters[ FText_paoPrintfLetters[ nLetterArrayIndex ].uFntLetterIdx ];
		if( ( FText_paoPrintfLetters[ nLetterArrayIndex ].hFont == pFont->oHandle ) && ( pFontLetter->uTexurePage == nTexturePageIdx ) ) 
		{
			//calculate the character coordinates
			f32 fLetterUpperLeftX, fLetterUpperRightX, fLetterLowerLeftX, fLetterLowerRightX;
			f32 fLetterUpperY, fLetterLowerY; //the y values will be the same between left and right
			ftext_CalculateLetterCoordinates( &FText_paoPrintfLetters[ nLetterArrayIndex ], 
				&fLetterUpperLeftX, &fLetterUpperRightX, &fLetterLowerLeftX, &fLetterLowerRightX,
				&fLetterUpperY, &fLetterLowerY );

			if ( uCurrentArrayVertCount + 4 > __MAX_VERTICES_PER_ARRAY )
			{
				u16 nCharacterVertsThisArray = (nCharactersToRender - nCharactersPrepped) * 4;

				if ( nCharacterVertsThisArray > __MAX_VERTICES_PER_ARRAY )
				{
					nCharacterVertsThisArray = __MAX_VERTICES_PER_ARRAY;
				}

				// Encode the start of this array
				*pPush = D3DPUSH_ENCODE( D3DPUSH_NOINCREMENT_FLAG|D3DPUSH_INLINE_ARRAY, (nCharacterVertsThisArray * __DWORDS_PER_VERTEX) );
				pPush++;

				// Reset the vertex count
				uCurrentArrayVertCount = 0;
			}

			nD3DColor = (FText_paoPrintfLetters[ nLetterArrayIndex ].oColor.nAlpha<<24) | (FText_paoPrintfLetters[ nLetterArrayIndex ].oColor.nRed<<16) | (FText_paoPrintfLetters[ nLetterArrayIndex ].oColor.nGreen<<8) | FText_paoPrintfLetters[ nLetterArrayIndex ].oColor.nBlue;

			// VERTEX 0 //

			// Write the position
			*((f32*)pPush) = fLetterUpperLeftX;
			pPush++;
			*((f32*)pPush) = fLetterUpperY;
			pPush++;
			*((f32*)pPush) = 1.f;
			pPush++;

			// Write the diffuse color
			*((D3DCOLOR*)pPush) = nD3DColor;
			pPush++;

			// Write the texture ST
			*((f32*)pPush) = pFontLetter->fUVUpperLeftX;
			pPush++;
			*((f32*)pPush) = pFontLetter->fUVUpperLeftY;
			pPush++;

			// VERTEX 1 //

			// Write the position
			*((f32*)pPush) = fLetterLowerLeftX;
			pPush++;
			*((f32*)pPush) = fLetterLowerY;
			pPush++;
			*((f32*)pPush) = 1.f;
			pPush++;

			// Write the diffuse color
			*((D3DCOLOR*)pPush) = nD3DColor;
			pPush++;

			// Write the texture ST
			*((f32*)pPush) = pFontLetter->fUVUpperLeftX;
			pPush++;
			*((f32*)pPush) = pFontLetter->fUVLowerRightY;
			pPush++;

			// VERTEX 2 //

			// Write the position
			*((f32*)pPush) = fLetterLowerRightX;
			pPush++;
			*((f32*)pPush) = fLetterLowerY;
			pPush++;
			*((f32*)pPush) = 1.f;
			pPush++;

			// Write the diffuse color
			*((D3DCOLOR*)pPush) = nD3DColor;
			pPush++;

			// Write the texture ST
			*((f32*)pPush) = pFontLetter->fUVLowerRightX;
			pPush++;
			*((f32*)pPush) = pFontLetter->fUVLowerRightY;
			pPush++;

			// VERTEX 3 //

			// Write the position
			*((f32*)pPush) = fLetterUpperRightX;
			pPush++;
			*((f32*)pPush) = fLetterUpperY;
			pPush++;
			*((f32*)pPush) = 1.f;
			pPush++;
		
			// Write the diffuse color
			*((D3DCOLOR*)pPush) = nD3DColor;
			pPush++;

			// Write the texture ST
			*((f32*)pPush) = pFontLetter->fUVLowerRightX;
			pPush++;
			*((f32*)pPush) = pFontLetter->fUVUpperLeftY;
			pPush++;

			// Increment the vertex count
			uCurrentArrayVertCount += 4;

			// Increment the prep count
			nCharactersPrepped++;
		}

		nLetterArrayIndex++;
	}

	// Push the macros that finish things off
	*pPush++ = D3DPUSH_ENCODE( D3DPUSH_SET_BEGIN_END, 1 );
	*pPush++ = 0;

	FDX8_pDev->EndPush( pPush );

	#undef __DWORDS_PER_VERTEX
	#undef __MAX_VERTICES_PER_ARRAY
}
#endif



