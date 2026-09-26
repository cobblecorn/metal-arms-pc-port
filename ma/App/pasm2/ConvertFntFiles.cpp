//////////////////////////////////////////////////////////////////////////////////////
// ConvertFntFiles.cpp
//
// Author: Michael Starich   
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
// 10/10/01 ayale       Modified.
// 10/10/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "stdafx.h"
#include "fang.h"
#include "ConvertFntFiles.h"
#include "ErrorLog.h"
#include "fmath.h"
#include "fdata.h"
#include "fclib.h"
#include "fresload.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define _ERROR_HEADING								".FNT FILE COMPILER "

//// Default font data.
//
// Note: copy fdx8fontdata.h or fgcfontdata.h (generated in the current directory) to fang2\dx or fang2\gc.
#define _OUTPUT_DEFAULT_FONTDATA_CPP_DATA_TO_CUR_DIR	FALSE
#define _REMAP_FONT_CASES								TRUE

//
////

#define _SAFE_FREE( p )		free( p ); \
							p = NULL;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CConvertFntFile::CConvertFntFile()
{
	//zero out the FontHeader
	m_oPasmFont.pauWidth = NULL;
	m_oPasmFont.pauHeight = NULL;
	m_oPasmFont.paoFntLetters = NULL;
	memset( &m_oPasmFont, 0, sizeof( FDataFntFile_Font_t ) );

	m_pCompiledFile = NULL;
	m_nCompiledBytes = 0;

} // CConvertFntFile

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CConvertFntFile::~CConvertFntFile()
{
	FreeData();

} // ~CConvertFntFile

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CConvertFntFile::FreeData()
{
	_SAFE_FREE( m_oPasmFont.pauWidth );
	_SAFE_FREE( m_oPasmFont.pauHeight );
	_SAFE_FREE( m_oPasmFont.paoFntLetters );

	while( m_BucketLetterList.GetCount() ) {
		FDataFntFile_LetterBucket_t *pBucketLetter = (FDataFntFile_LetterBucket_t *) m_BucketLetterList.RemoveHead();
		free( pBucketLetter );
	}

	if( m_pCompiledFile ) {
		fang_Free( m_pCompiledFile );
		m_pCompiledFile = NULL;
	}
	m_nCompiledBytes = 0;

} // FreeData

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL CConvertFntFile::ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian/*=FALSE*/ )
{
	// Freeing any past allocations.
	FreeData();

	m_bConvertToBigEndian = bConvertToBigEndian;

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();
	CString oErrMsg;

	//// Open font definition file.
	//
	FILE *poFile = fopen( pFileInfo->GetFilePath(), "rb" );
	if( ! poFile )
	{
		oErrMsg.Format( "Error %u: Could not find font (\"%s\") definition file !!!\r\n", __LINE__, pFileInfo->GetFilePath() );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}
	//
	////

	//// Read font definition file.
	//
	char *pauBuffer = (char *)malloc( pFileInfo->GetLength() );
	if( ! pauBuffer )
	{
		fclose( poFile );
		oErrMsg.Format( "Error %u: Could not allocate enough memory for font definition file !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}
	fread( pauBuffer, pFileInfo->GetLength(), 1, poFile );
	fclose( poFile );
	//
	////

	fang_MemZero( &m_oPasmFont, sizeof( m_oPasmFont ) );
	fclib_strncpy( (char *)m_oPasmFont.oFntInfo.szName, pFileInfo->GetFileTitle(), FRES_NAMELEN );

	//read the first word of the file... 
	//if it's not the UNICODE MAGIC COOKIE, bail out
	u16* pnCookie = (u16*) pauBuffer;
	if( *pnCookie != 0xFEFF ) {
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: This Font file is NOT a unicode File.  Aborting Compile !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}

	wchar_t *pauWBuffer,*pszTemp, *pszTemp2;
	u32 uIndex;
	pauWBuffer = ( wchar_t *) pauBuffer;

	//// # Texture pages.
	//
	pszTemp = wcsstr( pauWBuffer, L"\r\nCOMMAND FONT_SET_NUM_TEXTURE_PAGES" );
	if( ! pszTemp )
	{
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not find \"COMMAND FONT_SET_NUM_TEXTURE_PAGES\" in font definition file !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}
	pszTemp += wcslen( L"\r\nCOMMAND FONT_SET_NUM_TEXTURE_PAGES" );
	if( 1 != swscanf( pszTemp, L"%u", &uIndex ) )
	{
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not read \"COMMAND FONT_SET_NUM_TEXTURE_PAGES\" in font definition file !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}
	//
	////

	//// #Letters.
	//
	pszTemp = wcsstr( pauWBuffer, L"\r\nCOMMAND FONT_NUMBER_LETTERS" );
	if( ! pszTemp )
	{
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not find \"COMMAND FONT_NUMBER_LETTERS\" in font definition file !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}
	pszTemp += wcslen( L"\r\nCOMMAND FONT_NUMBER_LETTERS" );
	if( 1 != swscanf( pszTemp, L"%u", &m_oPasmFont.oFntInfo.uFntLetters ) )
	{
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not read \"COMMAND FONT_NUMBER_LETTERS\" in font definition file !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}

	//now, create the font letter array
	m_oPasmFont.paoFntLetters = ( FDataFntFile_Letter_t *) malloc( m_oPasmFont.oFntInfo.uFntLetters * sizeof( FDataFntFile_Letter_t ) );
	if( !m_oPasmFont.paoFntLetters )
	{
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not allocate memory for %d font structures !!!\r\n", __LINE__, m_oPasmFont.oFntInfo.uFntLetters );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}

	//
	////

	//// Size.
	//
	pszTemp = wcsstr( pauWBuffer, L"\r\nCOMMAND FONT_SET_SIZE" );
	if( ! pszTemp )
	{
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not find \"COMMAND FONT_SET_SIZE\" in font definition file !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}
	pszTemp += wcslen( L"\r\nCOMMAND FONT_SET_SIZE" );
	if( 2 != swscanf( pszTemp, L"%f %f", &m_oPasmFont.oFntInfo.fWidth, &m_oPasmFont.oFntInfo.fHeight ) )
	{
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not read \"COMMAND FONT_SET_SIZE\" in font definition file !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}
	//
	////

	//// Italic slant.
	//
	pszTemp = wcsstr( pauWBuffer, L"\r\nCOMMAND FONT_SET_ITALIC_SLANT" );
	if( ! pszTemp )
	{
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not find \"COMMAND FONT_SET_ITALIC_SLANT\" in font definition file !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}
	pszTemp += wcslen( L"\r\nCOMMAND FONT_SET_ITALIC_SLANT" );
	if( 1 != swscanf( pszTemp, L"%f", &m_oPasmFont.oFntInfo.fItalicSlant ) )
	{
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not read \"COMMAND FONT_SET_ITALIC_SLANT\" in font definition file !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}
	//
	////

	//// Vertical ruler.
	//
	pszTemp = wcsstr( pauWBuffer, L"\r\nCOMMAND FONT_SET_VERTICAL_RULER" );
	if( ! pszTemp )
	{
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not find \"COMMAND FONT_SET_VERTICAL_RULER\" in font definition file !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}
	pszTemp += wcslen( L"\r\nCOMMAND FONT_SET_VERTICAL_RULER" );
	if( 1 != swscanf( pszTemp, L"%f", &m_oPasmFont.oFntInfo.fVerticalRuler ) )
	{
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not read \"COMMAND FONT_SET_VERTICAL_RULER\" in font definition file !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}
	//
	////

	//// Word spacing.
	//
	pszTemp = wcsstr( pauWBuffer, L"\r\nCOMMAND FONT_SET_WORD_SPACING" );
	if( ! pszTemp )
	{
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not find \"COMMAND FONT_SET_WORD_SPACING\" in font definition file !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}
	pszTemp += wcslen( L"\r\nCOMMAND FONT_SET_WORD_SPACING" );
	if( 1 != swscanf( pszTemp, L"%f", &m_oPasmFont.oFntInfo.fWordSpacing ) )
	{
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not read \"COMMAND FONT_SET_WORD_SPACING\" in font definition file !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}
	//
	////

	//// Letter spacing.
	//
	pszTemp = wcsstr( pauWBuffer, L"\r\nCOMMAND FONT_SET_LETTER_SPACING" );
	if( ! pszTemp )
	{
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not find \"COMMAND FONT_SET_LETTER_SPACING\" in font definition file !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}
	pszTemp += wcslen( L"\r\nCOMMAND FONT_SET_LETTER_SPACING" );
	if( 1 != swscanf( pszTemp, L"%f", &m_oPasmFont.oFntInfo.fLetterSpacing ) )
	{
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not read \"COMMAND FONT_SET_LETTER_SPACING\" in font definition file !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}
	//
	////

	//// Load font textures.
	//
	m_oPasmFont.pauWidth = (u32 *)malloc( uIndex * sizeof( u32 ) );
	m_oPasmFont.pauHeight = (u32 *)malloc( uIndex * sizeof( u32 ) );
	if( ! ( m_oPasmFont.pauWidth && m_oPasmFont.pauHeight ) )
	{
		FreeData();
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not allocate memory necessary.", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}

	wchar_t szTemp[ 256 + 1 ];
	pszTemp = wcsstr( pauWBuffer, L"\r\nLOAD \"" );
	for( m_oPasmFont.oFntInfo.uTexPages = 0; m_oPasmFont.oFntInfo.uTexPages < uIndex; ++m_oPasmFont.oFntInfo.uTexPages )
	{
		//// Find texture name.
		//
		if( ! pszTemp )
		{
			FreeData();
			_SAFE_FREE( pauBuffer );
			oErrMsg.Format( "Error %u: Could not find \"LOAD\" for texture page #%u in font definition file !!!\r\n", __LINE__, m_oPasmFont.oFntInfo.uTexPages );
			rErrorLog.WriteErrorHeader( _ERROR_HEADING );
			rErrorLog.WriteErrorLine( oErrMsg );
			return FALSE;
		}
		pszTemp += wcslen( L"\r\nLOAD \"" );

		fang_MemZero( szTemp, sizeof( szTemp ) );
		wcsncpy( szTemp, pszTemp, FRES_NAMELEN );
		//fclib_strncpy( (char *)szTemp, pszTemp, FRES_NAMELEN ); //commented out by RAF
		pszTemp2 = wcsstr( szTemp, L"\"" );
		if( pszTemp2 )
		{
			*pszTemp2 = 0;
		}
		pszTemp += ( wcslen( szTemp ) + 1 );
		if( 2 != swscanf( pszTemp, L"%u %u", &m_oPasmFont.pauWidth[ m_oPasmFont.oFntInfo.uTexPages ], &m_oPasmFont.pauHeight[ m_oPasmFont.oFntInfo.uTexPages ] ) )
		{
			FreeData();
			_SAFE_FREE( pauBuffer );
			oErrMsg.Format( "Error %u: Could not read \"LOAD\" for texture page #%u in font definition file !!!\r\n", __LINE__, m_oPasmFont.oFntInfo.uTexPages );
			rErrorLog.WriteErrorHeader( _ERROR_HEADING );
			rErrorLog.WriteErrorLine( oErrMsg );
			return FALSE;
		}
		//
		////

		pszTemp = wcsstr( pszTemp, L"\r\nLOAD \"" );
	}
	//
	////

	//// Find letters.
	//
	wchar_t *pszLetters = wcsstr( pauWBuffer, L"\r\nTEXT \"" );
	if( ! pszLetters )
	{
		FreeData();
		_SAFE_FREE( pauBuffer );
		oErrMsg.Format( "Error %u: Could not find \"TEXT\" in font definition file !!!\r\n", __LINE__ );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING );
		rErrorLog.WriteErrorLine( oErrMsg );
		return FALSE;
	}
	pszLetters += wcslen( L"\r\nTEXT \"" );
	//
	////

	//// Letter Stats.
	//
	struct
	{
		u16 auUV[ 8 ];
		u8 uWidth, uHeight, uTexturePage, uOriginalIndex, uID;
		int nAlignVert;

	} oLetter;

	pszTemp = pauWBuffer;
	u16 uCharIndex;
	FDataFntFile_Letter_t *poFntLetter;
	for( uIndex = 0; uIndex < m_oPasmFont.oFntInfo.uFntLetters; ++uIndex )
	{
		// Find command.
		pszTemp = wcsstr( pszTemp, L"COMMAND FONT_SET_LETTER_INFO" );
		if( ! pszTemp )
		{
			FreeData();
			_SAFE_FREE( pauBuffer );
			oErrMsg.Format( "Error %u: Could not find \"COMMAND FONT_SET_LETTER_INFO\" in font definition file for letter #%u !!!\r\n", __LINE__, uIndex );
			rErrorLog.WriteErrorHeader( _ERROR_HEADING );
			rErrorLog.WriteErrorLine( oErrMsg );
			return FALSE;
		}
		pszTemp += wcslen( L"COMMAND FONT_SET_LETTER_INFO" );

		// Read arguments.
		if( 14 != swscanf( pszTemp, L"%u %u %u %u %u %u %u %u %u %u %d %u %u %u\r\n", &oLetter.auUV[ 0 ], &oLetter.auUV[ 1 ], &oLetter.auUV[ 2 ], &oLetter.auUV[ 3 ], &oLetter.auUV[ 4 ], &oLetter.auUV[ 5 ], &oLetter.auUV[ 6 ], &oLetter.auUV[ 7 ], &oLetter.uWidth, &oLetter.uHeight, &oLetter.nAlignVert, &oLetter.uTexturePage, &oLetter.uOriginalIndex, &oLetter.uID ) )
		{
			FreeData();
			_SAFE_FREE( pauBuffer );
			oErrMsg.Format( "Error %u: Could not read \"COMMAND FONT_SET_LETTER_INFO\" arguments in font definition file for letter #%u !!!\r\n", __LINE__, uIndex );
			rErrorLog.WriteErrorHeader( _ERROR_HEADING );
			rErrorLog.WriteErrorLine( oErrMsg );
			return FALSE;
		}

		// Parse arguments.
		uCharIndex = pszLetters[ oLetter.uOriginalIndex ];
		poFntLetter = &m_oPasmFont.paoFntLetters[ uIndex ];
		if( !poFntLetter )
		{
			FreeData();
			_SAFE_FREE( pauBuffer );
			oErrMsg.Format( "Error %u: Could not allocate memory necessary.", __LINE__ );
			rErrorLog.WriteErrorHeader( _ERROR_HEADING );
			rErrorLog.WriteErrorLine( oErrMsg );
			return FALSE;
		}
		fang_MemZero( poFntLetter, sizeof( FDataFntFile_Letter_t ) );

		poFntLetter->uTexurePage = oLetter.uTexturePage;
		poFntLetter->fWidth = (f32)oLetter.uWidth;
		poFntLetter->fHeight = (f32)oLetter.uHeight;
		poFntLetter->fVertAlign = (f32)oLetter.nAlignVert;

		poFntLetter->fUVUpperLeftX  = (f32)oLetter.auUV[ 2 ] / (f32)m_oPasmFont.pauWidth[  poFntLetter->uTexurePage ];
		poFntLetter->fUVUpperLeftY  = (f32)oLetter.auUV[ 3 ] / (f32)m_oPasmFont.pauHeight[ poFntLetter->uTexurePage ];
		poFntLetter->fUVLowerRightX = (f32)oLetter.auUV[ 6 ] / (f32)m_oPasmFont.pauWidth[  poFntLetter->uTexurePage ];
		poFntLetter->fUVLowerRightY = (f32)oLetter.auUV[ 7 ] / (f32)m_oPasmFont.pauHeight[ poFntLetter->uTexurePage ];

		//create a BucketLetter Structure for this character
		FDataFntFile_BucketLetter_t *pBucketLetter = (FDataFntFile_BucketLetter_t *) malloc( sizeof( FDataFntFile_BucketLetter_t ) );
		if( !pBucketLetter )
		{
			FreeData();
			_SAFE_FREE( pauBuffer );
			oErrMsg.Format( "Error %u: Could not create a FDataFntFile_BucketLetter_t struct!!!\r\n", __LINE__ );
			rErrorLog.WriteErrorHeader( _ERROR_HEADING );
			rErrorLog.WriteErrorLine( oErrMsg );
			return FALSE;
		}
		pBucketLetter->uCharCode = uCharIndex;
		pBucketLetter->uFntLetterIndex = uIndex;
		m_BucketLetterList.AddTail( pBucketLetter );
	}

	_SAFE_FREE( pauBuffer );

	//
	////

	//// Calc font vert and horz align.
	//
	f32 fFontTop, fFontBottom, fFontTopMax = 0, fFontBottomMax = 0;

	for( uIndex = 0; uIndex < m_oPasmFont.oFntInfo.uFntLetters; ++uIndex )
	{
		fFontTop = m_oPasmFont.paoFntLetters[ uIndex ].fVertAlign - m_oPasmFont.paoFntLetters[ uIndex ].fHeight;
		fFontBottom = m_oPasmFont.paoFntLetters[ uIndex ].fVertAlign;

		// Keep highest top and lowest bottom.
		if( fFontBottomMax < fFontBottom )
		{
			fFontBottomMax = fFontBottom;
		}
		if( fFontTopMax > fFontTop )
		{
			fFontTopMax = fFontTop;
		}
	}

	for( uIndex = 0; uIndex < m_oPasmFont.oFntInfo.uFntLetters; ++uIndex )
	{
		// Calc new vertical alignment from top.
		m_oPasmFont.paoFntLetters[ uIndex ].fVertAlign = ( ( m_oPasmFont.paoFntLetters[ uIndex ].fVertAlign - m_oPasmFont.paoFntLetters[ uIndex ].fHeight ) - fFontTopMax );

		// Calc horizontal alignment from widest letter.
		m_oPasmFont.paoFntLetters[ uIndex ].fHorzAlign = ( ( m_oPasmFont.oFntInfo.fWidth - m_oPasmFont.paoFntLetters[ uIndex ].fWidth ) * 0.5f );

		// Calc italic slant.
		m_oPasmFont.paoFntLetters[ uIndex ].fItalicPositiveSlantTop    = 1 - ( m_oPasmFont.paoFntLetters[ uIndex ].fVertAlign / m_oPasmFont.oFntInfo.fHeight );
		m_oPasmFont.paoFntLetters[ uIndex ].fItalicPositiveSlantBottom = 1 - ( ( m_oPasmFont.paoFntLetters[ uIndex ].fVertAlign + m_oPasmFont.paoFntLetters[ uIndex ].fHeight ) / m_oPasmFont.oFntInfo.fHeight );

		m_oPasmFont.paoFntLetters[ uIndex ].fItalicNegativeSlantTop    = - ( m_oPasmFont.paoFntLetters[ uIndex ].fVertAlign / m_oPasmFont.oFntInfo.fHeight );
		m_oPasmFont.paoFntLetters[ uIndex ].fItalicNegativeSlantBottom = - ( ( m_oPasmFont.paoFntLetters[ uIndex ].fVertAlign + m_oPasmFont.paoFntLetters[ uIndex ].fHeight ) / m_oPasmFont.oFntInfo.fHeight );
	}
	//
	////

#if _REMAP_FONT_CASES
	//// Remap Characters (Create Additional BucketLetter structures if need be)
	//
	for( uIndex = 'A'; uIndex <= 'Z'; ++uIndex ) {
		FDataFntFile_BucketLetter_t* pUppercaseBucketLetter = FindBucketLetter( uIndex );
		if( pUppercaseBucketLetter ) {
			//See if we can find a lower case index for this guy...
			FDataFntFile_BucketLetter_t* pLowercaseBucketLetter = FindBucketLetter( uIndex + 32 );
			if( !pLowercaseBucketLetter ) {
				//create a BucketLetter Structure for this character
				pLowercaseBucketLetter = (FDataFntFile_BucketLetter_t *) malloc( sizeof( FDataFntFile_BucketLetter_t ) );
				if( !pLowercaseBucketLetter )
				{
					FreeData();
					_SAFE_FREE( pauBuffer );
					oErrMsg.Format( "Error %u: Could not create a FDataFntFile_BucketLetter_t struct!!!\r\n", __LINE__ );
					rErrorLog.WriteErrorHeader( _ERROR_HEADING );
					rErrorLog.WriteErrorLine( oErrMsg );
					return FALSE;
				}
				pLowercaseBucketLetter->uCharCode = pUppercaseBucketLetter->uCharCode + 32; //lowercase ANSI index
				pLowercaseBucketLetter->uFntLetterIndex = pUppercaseBucketLetter->uFntLetterIndex;
				m_BucketLetterList.AddTail( pLowercaseBucketLetter );
			}
		}
	}
	
	for( uIndex = 'a'; uIndex <= 'z'; ++uIndex ) {
		FDataFntFile_BucketLetter_t* pLowercaseBucketLetter = FindBucketLetter( uIndex );
		if( pLowercaseBucketLetter ) {
			//See if we can find an upper case index for this guy...
			FDataFntFile_BucketLetter_t* pUppercaseBucketLetter = FindBucketLetter( uIndex - 32 );
			if( !pUppercaseBucketLetter ) {
				//create a BucketLetter Structure for this character
				pUppercaseBucketLetter = (FDataFntFile_BucketLetter_t *) malloc( sizeof( FDataFntFile_BucketLetter_t ) );
				if( !pUppercaseBucketLetter )
				{
					FreeData();
					_SAFE_FREE( pauBuffer );
					oErrMsg.Format( "Error %u: Could not create a FDataFntFile_BucketLetter_t struct!!!\r\n", __LINE__ );
					rErrorLog.WriteErrorHeader( _ERROR_HEADING );
					rErrorLog.WriteErrorLine( oErrMsg );
					return FALSE;
				}
				pUppercaseBucketLetter->uCharCode = pLowercaseBucketLetter->uCharCode - 32; //lowercase ANSI index
				pUppercaseBucketLetter->uFntLetterIndex = pLowercaseBucketLetter->uFntLetterIndex;
				m_BucketLetterList.AddTail( pUppercaseBucketLetter );
			}
		}
	}
#endif //_REMAP_FONT_CASES

	//AT this point, all the LetterBuckets we need have been created.  It's time
	//to run through the letterbucket list and calculate the total number of letters 
	//that will be in each bucket

	m_oPasmFont.oFntInfo.uBucketLetters = m_BucketLetterList.GetCount();

	POSITION TempPosition = m_BucketLetterList.GetHeadPosition();
	while( TempPosition ) {
		FDataFntFile_BucketLetter_t* pTempLetter = ( FDataFntFile_BucketLetter_t* ) m_BucketLetterList.GetNext( TempPosition );
		u32 uBucketIndex = pTempLetter->uCharCode & 0xFF; //we are only interested in the LSB
		m_oPasmFont.aoBuckets[ uBucketIndex ].nNumLettersInBucket++;
	}

	//now, it's time to run through all the buckets, see which ones
	//have letters in them, and compose the letter to bucket index array list.
	//Also set the nLetterOffsetIndex value
	u32 nBucketLetterOffsetIndex = 0;
	for( uIndex = 0; uIndex < FDATA_FNT_MAXBUCKETS_IN_CHAR_SET; uIndex++ ) {
		if( m_oPasmFont.aoBuckets[ uIndex ].nNumLettersInBucket > 0 ) {
			m_oPasmFont.oFntInfo.auLetterToBucketIndex[ uIndex ] = m_oPasmFont.oFntInfo.uBuckets;
			m_oPasmFont.aoBuckets[ uIndex ].nBucketLetterBaseIndex = nBucketLetterOffsetIndex;
			m_oPasmFont.oFntInfo.uBuckets++;
			nBucketLetterOffsetIndex += m_oPasmFont.aoBuckets[ uIndex ].nNumLettersInBucket;
		} else {
			m_oPasmFont.oFntInfo.auLetterToBucketIndex[ uIndex ] = FDATA_FNT_LETTERBUCKET_EMPTY;
		}
	}
	//Now, we know how many Letter Buckets we need to export,
	//we know how many Bucket Letters we need to export
	//we know how many FntLetters we need to export.

	//lets create the master image file...

	////////////////////////////////////////////////////////////////
	// Determine the size and addresses of data to allocate
	////////////////////////////////////////////////////////////////
	u32 nDataBytes = 0;

	u32 ADDR_FDataFnt =			FMATH_BYTE_ALIGN_UP( nDataBytes, 4 );
	nDataBytes =				ADDR_FDataFnt + sizeof( FDataFntFile_Font_t );

	u32 ADDR_FLetterBuckets =	FMATH_BYTE_ALIGN_UP( nDataBytes, 4 );
	nDataBytes =				ADDR_FLetterBuckets + ( sizeof( FDataFntFile_LetterBucket_t ) * m_oPasmFont.oFntInfo.uBuckets );

	u32 ADDR_FBucketLetters =	FMATH_BYTE_ALIGN_UP( nDataBytes, 4 );
	nDataBytes =				ADDR_FBucketLetters + ( sizeof( FDataFntFile_BucketLetter_t ) * m_oPasmFont.oFntInfo.uBucketLetters );

	u32 ADDR_FFntLetters =		FMATH_BYTE_ALIGN_UP( nDataBytes, 4 );
	nDataBytes =				ADDR_FFntLetters + ( sizeof( FDataFntFile_Letter_t ) * m_oPasmFont.oFntInfo.uFntLetters );

	// Round it up
	nDataBytes = FMATH_BYTE_ALIGN_UP( nDataBytes, 4 );

	// Allocate memory to contain export data
	u8 *pExportData = (u8 *)fang_MallocAndZero( nDataBytes, FCLASS_BYTE_ALIGN );
	if ( pExportData == NULL ) 
	{
		DEVPRINTF( "CConvertFntFile::ConvertFile(): Could not allocate %u bytes.\n", nDataBytes );
		return FALSE;
	}

	u8 *pDataEnd = pExportData + nDataBytes;

	////////////////////////////////////////////////////////////////
	// Fill in export data memory, and change Endian if needed
	////////////////////////////////////////////////////////////////

	///////////////////////////////
	// PACK THE FDataFntFile_Font_t
	FDataFntFile_Font_t *pFontHeader = (FDataFntFile_Font_t *)(pExportData + ADDR_FDataFnt);
	memcpy(pFontHeader, &m_oPasmFont.oFntInfo, sizeof( FDataFntFile_Font_t ) );

	//FIXUP THE POINTERS->OFFSETS
	pFontHeader->paoLetterBuckets = ( FDataFntFile_LetterBucket_t *)ADDR_FLetterBuckets;
	pFontHeader->paoBucketLetters = ( FDataFntFile_BucketLetter_t *)ADDR_FBucketLetters;
	pFontHeader->paoFntLetters = ( FDataFntFile_Letter_t *)ADDR_FFntLetters;

	///////////////////////////////////////////////////////
	// PACK THE FDataFntFile_Font_t::paoLetterBuckets array
	FDataFntFile_LetterBucket_t *paoLetterBuckets = (FDataFntFile_LetterBucket_t *)(pExportData + ADDR_FLetterBuckets);
	for( uIndex = 0; uIndex < FDATA_FNT_MAXBUCKETS_IN_CHAR_SET; uIndex++ ){
		u32 uExportedIndex = pFontHeader->auLetterToBucketIndex[ uIndex ];
		if( uExportedIndex != FDATA_FNT_LETTERBUCKET_EMPTY ) {
			memcpy( &paoLetterBuckets[ uExportedIndex ], &m_oPasmFont.aoBuckets[ uIndex ], sizeof( FDataFntFile_LetterBucket_t ) );
			if( bConvertToBigEndian ) {
				paoLetterBuckets[ uExportedIndex ].ChangeEndian();
			}
		}
	}

	///////////////////////////////////////////////////////
	// PACK THE FDataFntFile_Font_t::paoBucketLetters array
	FDataFntFile_BucketLetter_t *paoBucketLetters = (FDataFntFile_BucketLetter_t *)(pExportData + ADDR_FBucketLetters);
	for( uIndex = 0; uIndex < FDATA_FNT_MAXBUCKETS_IN_CHAR_SET; uIndex++ ){
		u32 uExportedIndex = pFontHeader->auLetterToBucketIndex[ uIndex ];
		if( uExportedIndex != FDATA_FNT_LETTERBUCKET_EMPTY ) {
			//search through the list and find all the letters that have a LSB of uIndex
			u32 uBucketLetterIndex = m_oPasmFont.aoBuckets[ uIndex ].nBucketLetterBaseIndex;
			POSITION TempPosition = m_BucketLetterList.GetHeadPosition();
			while( TempPosition ) {
				FDataFntFile_BucketLetter_t* pTempLetter = ( FDataFntFile_BucketLetter_t* ) m_BucketLetterList.GetNext( TempPosition );
				if( ( pTempLetter->uCharCode & 0xFF ) == uIndex ) {
					//This letter belongs to this bucket.
					memcpy( &paoBucketLetters[ uBucketLetterIndex ], pTempLetter, sizeof( FDataFntFile_BucketLetter_t ) );
					if( bConvertToBigEndian ) {
						paoBucketLetters[ uBucketLetterIndex ].ChangeEndian();
					}

					uBucketLetterIndex++;
				}
			}
		}
	}

	////////////////////////////////////////////////////
	// PACK THE FDataFntFile_Font_t::paoFntLetters array
	FDataFntFile_Letter_t *paoFntLetters = (FDataFntFile_Letter_t *)(pExportData + ADDR_FFntLetters);
	for( uIndex = 0; uIndex < pFontHeader->uFntLetters; uIndex++ ){
		memcpy( &paoFntLetters[ uIndex ], &m_oPasmFont.paoFntLetters[ uIndex ], sizeof( FDataFntFile_Letter_t ) );
		if( bConvertToBigEndian ) {
			paoFntLetters[ uIndex ].ChangeEndian();
		}
	}

	if( bConvertToBigEndian ) {
		pFontHeader->ChangeEndian();
	}

	//update our statistics variables
	m_pCompiledFile = pExportData;
	m_nCompiledBytes = nDataBytes;

	return TRUE;

} // ConvertFile

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

u32 CConvertFntFile::GetSizeOfConvertedFile()
{
	if( !m_nCompiledBytes ) {
		return 0;
	}
	return m_nCompiledBytes;
} // GetSizeOfConvertedFile

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL CConvertFntFile::WriteConvertedFile( cchar *pszFilename, FILE *pFileStream/*=NULL*/ )
{
	if( !m_nCompiledBytes ) {
		return FALSE;
	}
	BOOL bCloseFile = FALSE;
	if( !pFileStream ) {
		if( !pszFilename ) {
			// invalid filename
			return FALSE;
		}
		pFileStream = _tfopen( pszFilename, _T("wb") );
		if( !pFileStream ) {
			return FALSE;
		}
		bCloseFile = TRUE;
	}
	// write out our file
	fwrite( m_pCompiledFile, m_nCompiledBytes, 1, pFileStream );

	// close our file
	if( bCloseFile ) {
		fclose( pFileStream );
	}

#if( _OUTPUT_DEFAULT_FONTDATA_CPP_DATA_TO_CUR_DIR )
	//// Open the output file.
	//
	FILE *pFileStream2;
	if( m_bConvertToBigEndian )
	{
		pFileStream2 = fopen( "fgcfontdata.h", "wb" );
	}
	else
	{
		pFileStream2 = fopen( "fdx8fontdata.h", "wb" );
	}
	if( ! pFileStream2 )
	{
		return FALSE;
	}
	//
	////

	//// Write out our file.
	//
	FDataFntFile_Font_t *pFontHeader = (FDataFntFile_Font_t *)( m_pCompiledFile );
	fang_MemZero( pFontHeader->szName, sizeof( pFontHeader->szName ) );

	for( u32 uCount = 0; uCount < m_nCompiledBytes; uCount++ )
	{
		fprintf( pFileStream2, "0x%02x,%s", (u8 *)m_pCompiledFile[ uCount ], ( ( 0 == ( ( uCount + 1 ) % 20 ) ) ? "\r\n" : " " ) );
	}
	//
	////

	// close our file
	fclose( pFileStream2 );
#endif

	return TRUE;
} // WriteConvertedFile


FDataFntFile_BucketLetter_t* CConvertFntFile::FindBucketLetter( u16 uCharCode ){
	//Search through the Bucket Letter list and return a bucket if one exists...
	FDataFntFile_BucketLetter_t *pRetLetter = NULL;
	POSITION TempPosition = m_BucketLetterList.GetHeadPosition();
	while( TempPosition ) {
		FDataFntFile_BucketLetter_t* pTempLetter = ( FDataFntFile_BucketLetter_t* ) m_BucketLetterList.GetNext( TempPosition );
		if( pTempLetter->uCharCode == uCharCode ) {
			pRetLetter = pTempLetter;
			break;
		}
	}

	return pRetLetter;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
