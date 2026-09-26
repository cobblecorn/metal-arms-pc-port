//////////////////////////////////////////////////////////////////////////////////////
// ftext.cpp
//
// Author: Albert Yale
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
// 08/22/01 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <stdio.h>
#include "fang.h"
#include "ftext.h"
#include "ftex.h"
#include "ffile.h"
#include "fvtxpool.h"
#include "fresload.h"
#include "floop.h"
#include "fxfm.h"
#include "fviewport.h"
#include "fclib.h"
#include "fdraw.h"
#include "frenderer.h"

#if FANG_PLATFORM_GC
#include "fgcviewport.h"
#include <wchar.h> //for _vsnwprintf
#endif
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if( ! ( FTEXT_STUB_EVERYTHING_OUT ) )

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define _VERTS_USAGE_STATS								TRUE
#define _FTEXT_SHOW_TEXT_SAFE_AREA_OUTLINE_640_480		FALSE
#define _FTEXT_STUB_ONLY_RUNTIME_OUT					FALSE
#define _BLINKING_TYPE_COUNT							11
#define _PHYSICAL_DEVICE_ASPECT_RATIO_HoW				( 3.0f / 4.0f )
#define _PHYSICAL_DEVICE_ASPECT_RATIO_WoH				( 4.0f / 3.0f )

#define _GC_FRAMEBUFFER_WIDTH							( 512.f )
#define _GC_EXTERNAL_DISPLAY_WIDTH						( 640.f )
#define _GC_FRAMEBUFFER_HEIGHT							( 480.f )

//This is a bit of a hack -- it would be better to query the frame buffer size and calculate this
//instead of hardcoding to a particular GC resolution....
#if FANG_PLATFORM_GC
#define _FRAMEBUFFER_X_SCALEFACTOR						( _GC_FRAMEBUFFER_WIDTH / _GC_EXTERNAL_DISPLAY_WIDTH )
#else
#define _FRAMEBUFFER_X_SCALEFACTOR						( 1.f )
#endif

#if 1
#define _OVERFLOW_MAX_ERRORS							( 5 )
#else
#define _OVERFLOW_MAX_ERRORS							( 5000000 )
#endif


extern void ftext_DrawTexturePageCharacters( FDataFntFile_Font_t *pFont, u32 nTexturePageIdx );


void ftext_CalculateLetterCoordinates( FText_FTextPrintfLetter_t *pLetterStruct, 
										 f32 *pfUpperLeftX, f32 *pfUpperRightX, f32 *pfLowerLeftX, f32 *pfLowerRightX,
										 f32 *pfUpperY, f32 *pfLowerY );

FText_FTextPrintfLetter_t *FText_paoPrintfLetters;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/*
Possible optimizations:

- Reuse the same vert buffer between areas and text.
- ASM optimization for casting int -> f32.

Ideas:

- Write seperate text pre-parser application to validate escape codes and count frequency of each letter (for use in fontomatic).
- optimize vert buffers according to letter frequency in text to output.
- optimize vert buffers according to letter count per texture pages.
*/
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef enum
{
	_FTEXT_PRINTF_AREA_ONLY =	 0,
	_FTEXT_PRINTF_XY_ONLY,
	_FTEXT_PRINTF_AREA_AND_XY,
	_FTEXT_PRINTF_DEBUG_XY_ONLY

} _FTextPrintf_e;

struct _FTextArea_t
{
	_FTextArea_t *poAreaNext;

	CFTexInst oTexInst;
	BOOL bAreaDrawn, bBorderDrawn;
	f32 fUnitCursorY;
	CFRect2D oVpRect;		// Bounding rect of viewport that was active when the draw was requested
	CFVec2   oVpRes;		// Resolution (size) of said viewport

	FTextArea_t oAttributes;
};

typedef struct
{
	f32 fTimer; // In seconds.
	f32 fAlpha;

} _FTextBlinkingAlphas_t;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Module.
static BOOL _bModuleStarted = FALSE;
static BOOL _bModuleInstalled = FALSE;
static BOOL _bStaticFontFixedup = FALSE;

// Areas.
static _FTextArea_t *_poAreasHead;
static _FTextArea_t _oAreaDefault;

// FText Printf Letter structures
static u32 _nLettersUsed;
static u32 _nLettersInCurrentLine;
static u32 _nLettersInCurrentWord;

// Fonts.
static FDataFntFile_Font_t *_poFontsHead;

// View.
static CFXfm _oCamera;
static FViewport_t *_poViewportPrevious;
static FViewport_t *_poViewportDefault;

// Blinking.
static BOOL _bBlinkingAlphasNeedUpdate = TRUE;
static _FTextBlinkingAlphas_t _aoBlinkingAlphas[ _BLINKING_TYPE_COUNT ];

// Errors reporting.
static u32 _uTextBufferOverflowErrors;

#if _VERTS_USAGE_STATS
// Usage statistics.
static u32 _uStatVertsUsedMaxSingleDraw, _uStatVertsUsedCurrentDraw, _uStatVertsAllocatedTotal, _uStatVertsUsedTotal, _uStatDrawsCalled;
#endif

// Temp.
static CFColorRGBA _oTempColor;
static u32 _uIndex, _uTemp, _uCount, _uChar;
static f32 _fTempItalicSlantUnscaled, _fTempItalicSlant, _fTempSpaceCharSize, _fTempFontScale, _fTempOriginalScale,
		   _fTempX, _fTempY, _fTempFixupX1, _fTempFixupY1, _fTempFixupX2, _fTempFixupY2,
		   _fTempVertOffset,
		   _fTempDeltaX, _fTempDeltaY,
		   _fTempCursorOriginX, _fTempCursorOriginY,
		   _fTempResX, _fTempResY,
		   _fTempLowerRightInnerEdgeX, _fTempLowerRightInnerEdgeY,
		   _fTempBaseAlpha, _fTempGeneralAlpha, _fTempBlinkingAlpha,
		   _fTempFrameBufferXRescaleFactor;

static char		*_pszTemp;
static wchar	*_pszWTemp;

static _FTextArea_t *_poTempArea;
static _FTextArea_t *_poTempArea2;
static FDataFntFile_Font_t *_poTempFont;
static FDataFntFile_Font_t *_poTempFont2;
static FDataFntFile_Letter_t *_poTempLetter;
static FTextHorzAlign_e _oTempHorzAlign;
static BOOL _bTempFixedWidthFont, _bTempItalicFont, _bTempOTOTexelPixelAlignment;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if( FANG_PLATFORM_WIN )
	static u8 _auDefaultFnt[] =
	{
	#include "dx\fdx8fontdata.h"
	};
	static u8 _auDefaultTga[] = // Lucida Sans Typewriter 10 pts.
	{
	#include "dx\fdx8fonttex.h"
	};
#elif( FANG_PLATFORM_XB )
	static u8 _auDefaultFnt[] =
	{
	#include "dx\fxbfontdata.h"
	};
	static u8 _auDefaultTga[] = // Verdana 14 pts.
	{
	#include "dx\fxbfonttex.h"
	};
#elif( FANG_PLATFORM_GC )
	#include "fgc.h"
	static u8 FGC_ALIGN(32) _auDefaultFnt[] =
	{
	#include "gc\fgcfontdata.h"
	};
	static u8 FGC_ALIGN(32) _auDefaultTga[] = // Verdana 14 pts.
	{
	#include "gc\fgcfonttex.h"
	};
//ARG - >>>>>
#elif( FANG_PLATFORM_PS2 )
	#include "fps2.h"
//ARG - same as XB at the moment
	static u8 FPS2_ALIGN(32) _auDefaultFnt[] =
	{
	#include "ps2\fps2fontdata.h"
	};
	static u8 FPS2_ALIGN(128) _auDefaultTga[] = // Verdana 14 pts.
	{
	#include "ps2\fps2fonttex.h"
	};

	int _vsnprintf(char *string, int maxlen, const char *format, char *args)
	{
		//ARG - maxlen ignored
		vsprintf(string, format, args);
	}

	static wchar _spbuf[64];

	int _vsnwprintf(wchar *string, int maxlen, const wchar *format, va_list arglist)
	{
		//ARG - maxlen ignored

		//u_int x;

		unsigned int x;

		const wchar	*p = format;
		wchar	*d = string;
		wchar	*s;
		wchar	*sx;
		wchar 	c;
		wchar 	c2;	
		wchar 	filler;
		wchar 	field;

		while ( (c = *p++) )
		{
			if (c != '%')
			{
				*d++ = c; // anything but % is a regular character
				continue;
			}

			// c is %, get next

			c2 = *p++;

	/////////////////////////////

			if (c2 == 'l')
			{
				c2 = *p++; // get s or c
			}

	/////////////////////////////

			if (c2 == 'c') // append character
			{
				*d++ = (wchar)va_arg(arglist, wchar);
				continue;
			}

	/////////////////////////////

			if (c2 == 's') // append string
			{
				if (!(s = va_arg(arglist, wchar *) ) )
				{
					s = L"(null)"; // get arg from stack, incr ptr
				}
				for (; (c = *s); s++)
				{
					*d++ = c;
				}
				continue;
			}

	/////////////////////////////

			if (c2 == '.')
			{

#define MAX_CHARS 16

				char    string[MAX_CHARS];
				f32		value;
				int     i;
				int     len;

				value = (float)va_arg(arglist, double);	// get arg from stack, incr ptr

				// assumes .2f at moment, will need to add code if this breaks...

				p++; // get 2
				p++; // get f

				len = sprintf(string, "%.2f", value);

				for (i = 0; (i < len) && (i < MAX_CHARS); i++)
				{
					*d++ = (wchar)string[i];
				}

				continue;
			}

	/////////////////////////////

			if (c2 <= '9')
			{
				if (c2 < '0') // take care of %% situation plus some unknowns
				{
					*d++ = c2;
					continue;
				}
				if (c2 == '0') // %09d ; handle 0 to mean zero-padding
				{
					filler = '0';
					c2 = *p++;
				}
				else
				{			
					filler = ' ';
				}
				field = c2 - '0'; // %3d/%03d ; handle 3 to mean fieldwidth 3
				c2 = *p++;
			}
			else
			{
				field = 0;
			}

			// x = va_arg(arglist, u_int);
			x = va_arg(arglist, unsigned int); // get arg from stack, incr ptr
			s = &_spbuf[16];

			if (c2 == 'd' || c2 == 'u') // decimal conversion
			{
				do 
				{
					*--s = '0' | (wchar)(x % 10);

				} while ((x /= 10));
			}
			else
			{
				// hexadecimal conversion
				do
				{
					c = x & 0xF;
					*--s = c + (c < 10 ? '0': ('A' - 10) );

				} while ( (x >>= 4) );
			}
			
			if (field)
			{
				for (sx = d - (&_spbuf[16] - s) + field; d < sx; )
				{
					*d++ = filler;
				}
			}

			for (; (c = *s); s++)
			{
				*d++ = c;
			}
		} // end while ( (c = *p++) )

		*d = '\0';

		return d - string;
	}

	int _snwprintf(wchar *string, int maxlen, const wchar *format, ...)
	{
		int len;
		
		va_list arglist;

		va_start(arglist, format); // compute address on stack
		len = _vsnwprintf(string, maxlen, format, arglist);
		va_end(arglist);

		return len;
	}
//ARG - <<<<<
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _ResAreaDestroy( void *pResMem );
static void _ResFontDestroy( void *pResMem );

static void _ftext_UpdateBlinkingAlphas( void );
static void _ftext_PrintString( const _FTextPrintf_e oType,
							    const FTextAreaHandle_t ohArea,
								const f32 fX,
								const f32 fY,
								cwchar *pszWString,
								const f32 fZ=1.0f );
static void _InitializeWorkingVarsBasedOnPrintfType( const _FTextPrintf_e oType,
												     const FTextAreaHandle_t ohArea,
 													 const f32 fX,
													 const f32 fY );
static BOOL _ParseEscapeCode( const _FTextPrintf_e oType,
							  const f32 fX,
							  cwchar *pszWString);
static BOOL _ParseLetter( const _FTextPrintf_e oType,
						  const f32 fX,
						  cwchar *pszWString,
    					  const f32 fZ);
static void _CalculateHorizontalAlignmentOffset( const _FTextPrintf_e oType, const f32 fX, BOOL bWordWrapping = FALSE );
static void _ClearTexturePageLengthAttributes( u32 nClearMask );
static u16 _FindFntLetterIndex( FDataFntFile_Font_t *pFont, u16 uCharCode );

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL ftext_ModuleStartup( void )
{
	FASSERT_MSG( ! _bModuleStarted, "[ FTEXT ] Error: System already started !!!" );

	_bModuleStarted = TRUE;
	_bModuleInstalled = FALSE;

	_nLettersUsed = 0;

	return TRUE;

} // ftext_ModuleStartup

static void _ResAreaDestroy( void *pBase ) {
	
	FASSERT( _bModuleStarted );
	
	_FTextArea_t *pArea = _poAreasHead;
	while( pArea ) {
		if( pArea->poAreaNext <= pBase ) {
			pArea->poAreaNext = NULL;
		}
		pArea = pArea->poAreaNext;	
	};	
}

static void _ResFontDestroy( void *pBase ) {
	
	FASSERT( _bModuleStarted );
	
	FDataFntFile_Font_t * pFont = _poFontsHead;
	while( pFont ) {
		if( pFont->poFontNext <= pBase ) {
			pFont->poFontNext = NULL;
		}
		pFont = pFont->poFontNext;	
	};	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_ModuleShutdown( void )
{
	_bModuleStarted = FALSE;

} // ftext_ModuleShutdown

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FTextError_e ftext_Install( void )
{
	FASSERT_MSG( _bModuleStarted, "[ FTEXT ] Error: System not started !!!" );
	FASSERT_MSG( ! _bModuleInstalled, "[ FTEXT ] Error: System already installed !!!" );

	//// View.
	//
	_oCamera.Identity();
	_poViewportPrevious = NULL;
	_poViewportDefault = fviewport_Create();

	if( ! _poViewportDefault )
	{
		DEVPRINTF( "[ FTEXT ] Error %u: Could not create viewport !!!\n", __LINE__ );
		return FTEXT_ERROR;
	}

//We are going to create a framebuffer specific platform
//This is a super ugly hack, but it's gotta be done for proper looking text 
//on the GC...
#if FANG_PLATFORM_GC
	fviewport_InitOrtho2D( _poViewportDefault, 0.1f, 1000.0f, 0, 0, _GC_FRAMEBUFFER_WIDTH, _GC_FRAMEBUFFER_HEIGHT );
	//now, set a platform specific flag telling the GC system to NOT scale this viewport...
	_poViewportDefault->pViewportIS->nFlags |= FGCVIEWPORT_FLAG_DONT_APPLY_VIEWPORT_SCALE;
#else
	fviewport_InitOrtho2D( _poViewportDefault, 0.1f, 1000.0f );
#endif
	//
	////

	//// Blinking.
	//
	ftext_ResetBlinkTimers();
	//
	////

	//// Areas.
	//
	_poAreasHead = &_oAreaDefault;
	fang_MemZero( _poAreasHead, sizeof( _FTextArea_t ) );
	ftext_SetToDefaults( &( _poAreasHead->oAttributes ) );
#if FANG_PLATFORM_WIN
	_poAreasHead->oAttributes.oColorForeground.Set( 1.0f, 1.0f, 1.0f, 1.0f );
#else
	_poAreasHead->oAttributes.oColorForeground.Set( 0.70f, 0.70f, 0.70f, 1.0f );
#endif
	//
	////

	//// Fonts.
	//
	_poFontsHead = (FDataFntFile_Font_t *)_auDefaultFnt;

	_poFontsHead->poFontNext = NULL;
	_poFontsHead->oHandle = '0';

	if( ! _bStaticFontFixedup )
	{
		//// Fixup ( offsets -> pointers ).
		//
		_poFontsHead->paoLetterBuckets = (FDataFntFile_LetterBucket_t *)( (u32)_poFontsHead->paoLetterBuckets + (u32)_poFontsHead );
		_poFontsHead->paoBucketLetters = (FDataFntFile_BucketLetter_t *)( (u32)_poFontsHead->paoBucketLetters + (u32)_poFontsHead );
		_poFontsHead->paoFntLetters = (FDataFntFile_Letter_t *)( (u32)_poFontsHead->paoFntLetters + (u32)_poFontsHead );
		//
		////

		_bStaticFontFixedup = TRUE;
	}

	FResFrame_t ohTempResFrame = fres_GetFrame();

	//// Alloc mem for a texture resource pointer.
	//
	_poFontsHead->paoTexInsts = (CFTexInst *)fres_AllocAndZero( sizeof( CFTexInst ) );
	if( ! _poFontsHead->paoTexInsts )
	{
		DEVPRINTF( "[ FTEXT ] Error %u: Could not allocate memory necessary for default font.\n", __LINE__ );
		return FTEXT_ERROR;
	}
	//
	////

	//// Create the font texture.
	//
	_poFontsHead->paoTexInsts[ 0 ].SetTexDef( ftex_CreateTexture( (FTexInfo_t *)_auDefaultTga, (void *)( (u32)_auDefaultTga + sizeof( FTexInfo_t ) ), NULL ) );
	if( ! _poFontsHead->paoTexInsts[ 0 ].GetTexDef() )
	{
		fres_ReleaseFrame( ohTempResFrame );
		DEVPRINTF( "[ FTEXT ] Error %u : Could not create default font texture !!!\n", __LINE__ );
		return FTEXT_ERROR;
	}
	//
	////

	//// Alloc mem for font data.
	//
	_poFontsHead->pauTextLengthPerTexturePages = (u32 *)fres_AllocAndZero( sizeof( u32 ) );

	if( ! _poFontsHead->pauTextLengthPerTexturePages )
	{
		fres_ReleaseFrame( ohTempResFrame );
		DEVPRINTF( "[ FTEXT ] Error %u: Could not allocate memory necessary to load default font.\n", __LINE__ );
		return FTEXT_ERROR;
	}

	_poFontsHead->pauTextLengthPerTexturePages[ 0 ] = 0;
	//
	////
	//
	//// Fonts.

	// Allocate the letter structures 
	FText_paoPrintfLetters = ( FText_FTextPrintfLetter_t *)fres_Alloc( Fang_ConfigDefs.nText_MaxCharsPerFrame * sizeof( FText_FTextPrintfLetter_t ) );

	if( ! FText_paoPrintfLetters )
	{
		fres_ReleaseFrame( ohTempResFrame );
		DEVPRINTF( "[ FTEXT ] Error %u: Could not allocate memory necessary to print characters.\n", __LINE__ );
		return FTEXT_ERROR;
	}


#if _VERTS_USAGE_STATS
	//// Usage statistics.
	//
	_uStatVertsUsedMaxSingleDraw = _uStatVertsUsedTotal = _uStatDrawsCalled = 0;
	//
	////
#endif

	//allocate the memory for the single byte printf buffer
	_pszTemp = (char*) fres_AllocAndZero( Fang_ConfigDefs.nText_MaxCharsPerPrintf * sizeof( char ) );
	if( !_pszTemp ) {
		fres_ReleaseFrame( ohTempResFrame );
		DEVPRINTF( "[ FTEXT ] Error %u: Could not allocate memory necessary to buffer printf characters.\n", __LINE__ );
		return FTEXT_ERROR;
	}

	//allocate the memory for the wide string printf buffer
	_pszWTemp = (wchar*) fres_AllocAndZero( Fang_ConfigDefs.nText_MaxCharsPerPrintf * sizeof( wchar ) );
	if( !_pszWTemp ) {
		fres_ReleaseFrame( ohTempResFrame );
		DEVPRINTF( "[ FTEXT ] Error %u: Could not allocate memory necessary to buffer Widestring printf characters.\n", __LINE__ );
		return FTEXT_ERROR;
	}

	_uTextBufferOverflowErrors      = _OVERFLOW_MAX_ERRORS;

	_bModuleInstalled = TRUE;

#if 0
	DEVPRINTF( "[ FTEXT ] Static memory used: %u bytes\n",	sizeof( _bModuleStarted ) +
																sizeof( _bModuleInstalled ) +
																sizeof( _bStaticFontFixedup ) +
																sizeof( _poAreasHead ) +
																sizeof( _oAreaDefault ) +
																sizeof( _nLettersUsed ) +
																sizeof( _nLettersInCurrentWord ) +
																sizeof( _nLettersInCurrentLine ) +
																sizeof( FText_paoPrintfLetters ) +
																( Fang_ConfigDefs.nText_MaxCharsPerFrame * sizeof( FText_FTextPrintfLetter_t ) ) +
																sizeof( _poFontsHead ) +
																sizeof( _oCamera ) +
																sizeof( _poViewportPrevious ) +
																sizeof( _poViewportDefault ) +
																sizeof( _bBlinkingAlphasNeedUpdate ) +
																sizeof( _aoBlinkingAlphas ) +
																sizeof( _uTextBufferOverflowErrors ) +
	#if _VERTS_USAGE_STATS
																sizeof( _uStatVertsUsedMaxSingleDraw ) +
																sizeof( _uStatVertsUsedCurrentDraw ) +
																sizeof( _uStatVertsAllocatedTotal ) +
																sizeof( _uStatVertsUsedTotal ) +
																sizeof( _uStatDrawsCalled ) +
	#endif
																sizeof( _oTempColor ) +
																sizeof( _uIndex ) +
																sizeof( _uTemp ) +
																sizeof( _uCount ) +
																sizeof( _uChar ) +
																sizeof( _fTempItalicSlantUnscaled ) +
																sizeof( _fTempItalicSlant ) +
																sizeof( _fTempSpaceCharSize ) +
																sizeof( _fTempFontScale ) +
																sizeof( _fTempOriginalScale ) +
																sizeof( _fTempX ) +
																sizeof( _fTempY ) +
																sizeof( _fTempFixupX1 ) +
																sizeof( _fTempFixupY1 ) +
																sizeof( _fTempFixupX2 ) +
																sizeof( _fTempFixupY2 ) +
																sizeof( _fTempVertOffset ) +
																sizeof( _fTempDeltaX ) +
																sizeof( _fTempDeltaY ) +
																sizeof( _fTempCursorOriginX ) +
																sizeof( _fTempCursorOriginY ) +
																sizeof( _fTempResX ) +
																sizeof( _fTempResY ) +
																sizeof( _fTempLowerRightInnerEdgeX ) +
																sizeof( _fTempLowerRightInnerEdgeY ) +
																sizeof( _fTempBaseAlpha ) +
																sizeof( _fTempGeneralAlpha ) +
																sizeof( _fTempBlinkingAlpha ) +
																sizeof( _fTempFrameBufferXRescaleFactor ) +
																sizeof( _pszTemp ) + ( sizeof ( char ) * ( Fang_ConfigDefs.nText_MaxCharsPerPrintf + 1 ) ) + //_pszTemp
																sizeof( _pszWTemp ) + ( sizeof( wchar ) * ( Fang_ConfigDefs.nText_MaxCharsPerPrintf + 1 ) ) + //_pszWTemp
																sizeof( _poTempArea ) +
																sizeof( _poTempArea2 ) +
																sizeof( _poTempFont ) +
																sizeof( _poTempFont2 ) +
																sizeof( _poTempLetter ) +
																sizeof( _oTempHorzAlign ) +
																sizeof( _bTempFixedWidthFont ) +
																sizeof( _bTempItalicFont ) +
																sizeof( _bTempOTOTexelPixelAlignment ) +
																sizeof( _auDefaultFnt ) +
																sizeof( _auDefaultTga ) +
																sizeof( _poFontsHead->pauTextLengthPerTexturePages ) );
#endif

	return FTEXT_NO_ERROR;

} // ftext_Install

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_Uninstall( void )
{
#if _VERTS_USAGE_STATS
	//// Usage statistics.
	//
	if( _uStatDrawsCalled )
	{
//		DEVPRINTF( "[ FTEXT ] Stats: Max verts used in single draw: %5u\n", _uStatVertsUsedMaxSingleDraw );
//		DEVPRINTF( "[ FTEXT ] Stats: Total verts allocated:         %5u\n", _uStatVertsAllocatedTotal );
//		DEVPRINTF( "[ FTEXT ] Stats: Max verts used in single draw / Total verts allocated: %f\n", (f32)( (f32)_uStatVertsUsedMaxSingleDraw / (f32)_uStatVertsAllocatedTotal ) );
//		DEVPRINTF( "[ FTEXT ] Stats: Avg verts used in single draw: %5.0f\n", (f32)( (f32)_uStatVertsUsedTotal / (f32)_uStatDrawsCalled ) );
	}
	//
	////
#endif

	_bModuleInstalled = FALSE;

} // ftext_Uninstall

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL ftext_IsInstalled( void )
{
	return _bModuleInstalled;

} // ftext_IsInstalled

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct {
	f32 fSecsOn;
	f32 fSecsOff;
} _BlinkInfo_t;

static const _BlinkInfo_t _aBlinkInfo[_BLINKING_TYPE_COUNT-3] = {
	0.125f,		0.125f,	// 1/8 sec on, 1/8 sec off
	0.1875f,	0.0625f,// 3/16 sec on, 1/16 sec off
	0.25f,		0.25f,	// 1/4 sec on, 1/4 sec off
	0.375f,		0.125f,	// 3/8 sec on, 1/8 sec off
	0.375f,		0.375f,	// 3/8 sec on, 3/8 sec off
	0.5625f,	0.1875f,// 9/16 sec on, 3/16 sec off
	0.5f,		0.5f,	// 1/2 sec on, 1/2 sec off
	0.75f,		0.25f,	// 3/4 sec on, 1/4 sec off
};

void _ftext_UpdateBlinkingAlphas( void ) {
	u32 i, nIndex;
	f32 fDelta;
	
	// update from the 3rd to 2nd to last blink rate
	for( i=2; i < (_BLINKING_TYPE_COUNT-1); i++ ) {
		nIndex = i-2;

		// increase the timer
		_aoBlinkingAlphas[i].fTimer += FLoop_fRealPreviousLoopSecs;
		fDelta = _aoBlinkingAlphas[i].fTimer - _aBlinkInfo[nIndex].fSecsOn;
		if( fDelta >= 0.0f ) {
			// allow the proper off time
			if( fDelta >= _aBlinkInfo[nIndex].fSecsOff ) {
				// turn back on
				_aoBlinkingAlphas[i].fTimer = 0.0f;
				_aoBlinkingAlphas[i].fAlpha = 1.0f;
			} else {
				_aoBlinkingAlphas[i].fAlpha = 0.0f;
			}
		} else {
			// not enough time has past, leave on
			_aoBlinkingAlphas[i].fAlpha = 1.0f;
		}
	}

	_aoBlinkingAlphas[ 10 ].fTimer += ( FLoop_fRealPreviousLoopSecs * 0.2f );
	if( FMATH_2PI <= _aoBlinkingAlphas[ 10 ].fTimer ) {
		_aoBlinkingAlphas[ 10 ].fTimer = 0.0f;
		_aoBlinkingAlphas[ 10 ].fAlpha = 1.0f;
	} else {
		_aoBlinkingAlphas[ 10 ].fAlpha = ( ( fmath_Cos( FMATH_RAD2DEG( _aoBlinkingAlphas[ 10 ].fTimer ) ) + 1 ) * 0.5f );
	}
} // _ftext_UpdateBlinkingAlphas

void ftext_ResetBlinkTimers( void ) {

	_bBlinkingAlphasNeedUpdate = TRUE;
	fang_MemZero( _aoBlinkingAlphas, sizeof( _aoBlinkingAlphas ) );
	_aoBlinkingAlphas[ 0 ].fAlpha = 1.0f;// constant on
	_aoBlinkingAlphas[ 1 ].fAlpha = 0.0f;// constant off
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_SetToDefaults( FTextArea_t *poArea )
{
	FASSERT_MSG( poArea, "[ FTEXT ] Error: NULL pointer !!!" );

	fang_MemZero( poArea, sizeof( FTextArea_t ) );

	poArea->ohFont = '0';

	poArea->bVisible = TRUE;
	poArea->fTabSize = 4.0f;

	poArea->fAlpha = 1.0f;
	poArea->fLowerRightX = 1.0f;
	poArea->fLowerRightY = 1.0f * _PHYSICAL_DEVICE_ASPECT_RATIO_HoW;

	poArea->fUVLowerRightX = 1.0f;
	poArea->fUVLowerRightY = 1.0f;

	poArea->fNumberOfLines = 25.0f;

	poArea->oColorForeground.Set( 0.8f, 0.8f, 0.8f, 1.0f );

} // ftext_SetToDefaults

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FTextAreaHandle_t ftext_Create( const FTextArea_t *poArea, cchar *pszName )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );
	FASSERT_MSG( poArea,            "[ FTEXT ] Error: NULL pointer !!!" );

	FResFrame_t ohTempFrame = fres_GetFrame();
	
	// Create an fres resource...
	FResHandle_t hRes = fres_CreateWithCallback( NULL, NULL, _ResAreaDestroy );
	if( hRes == FRES_NULLHANDLE ) {
		DEVPRINTF( "ftext_Create(): Could not create ftext resource object.\n" );
		return 0;
	}

	_FTextArea_t *poNewArea = (_FTextArea_t *)fres_AllocAndZero( sizeof( _FTextArea_t ) );

	if( ! poNewArea )
	{
		DEVPRINTF( "[ FTEXT ] Error %u: Can't allocate memory for area !!!\n", __LINE__ );
		return 0;
	}
	// Set resource base pointer...
	fres_SetBase( hRes, poNewArea );

	//// Load area textures.
	//
	if( pszName )
	{
		poNewArea->oTexInst.SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, pszName ) );

		if( ! poNewArea->oTexInst.GetTexDef() )
		{
			fres_ReleaseFrame( ohTempFrame );
			DEVPRINTF( "[ FTEXT ] Error %u : Could not load area texture \"%s\" !!!\n", __LINE__, pszName );
			return 0;
		}
	}
	else
	{
		poNewArea->oTexInst.SetTexDef( NULL );
	}
	//
	////

	poNewArea->oAttributes = *poArea;

	// Add new area at the end of the list.
	_poTempArea = _poAreasHead;

	while( _poTempArea->poAreaNext )
	{
		_poTempArea = _poTempArea->poAreaNext;
	}

	_poTempArea->poAreaNext = poNewArea;

	return (FTextAreaHandle_t)poNewArea;

} // ftext_Create

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FTextArea_t *ftext_GetAttributes( const FTextAreaHandle_t ohArea )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

	if( ohArea )
	{
		return &( ((_FTextArea_t *)ohArea)->oAttributes );
	}
	else
	{
		return &( _poAreasHead->oAttributes );
	}

} // ftext_GetAttributes

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_SetAttributes( const FTextAreaHandle_t ohArea, const FTextArea_t *poArea )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );
	FASSERT_MSG( poArea,            "[ FTEXT ] Error: NULL pointer !!!" );

	if( ohArea )
	{
		((_FTextArea_t *)ohArea)->oAttributes = *poArea;
	}
	else
	{
		_poAreasHead->oAttributes = *poArea;
	}

} // ftext_SetAttributes

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

f32 ftext_GetLetterSpacing( const FDataFntFile_Handle_t ohFont )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );
	FASSERT_MSG( ohFont,            "[ FTEXT ] Error: NULL handle !!!" );

	////
	//
	_poTempFont = _poFontsHead;

	do
	{
		if( _poTempFont->oHandle == ohFont )
		{
			break;
		}

		_poTempFont = _poTempFont->poFontNext;

	} while( _poTempFont );

	FASSERT_MSG( _poTempFont, "[ FTEXT ] Error: NULL pointer (invalid font handle) !!!" );
	//
	////

	return _poTempFont->fLetterSpacing;

} // ftext_GetLetterSpacing

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

f32 ftext_GetWordSpacing( const FDataFntFile_Handle_t ohFont )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );
	FASSERT_MSG( ohFont,            "[ FTEXT ] Error: NULL handle !!!" );

	////
	//
	_poTempFont = _poFontsHead;

	do
	{
		if( _poTempFont->oHandle == ohFont )
		{
			break;
		}

		_poTempFont = _poTempFont->poFontNext;

	} while( _poTempFont );

	FASSERT_MSG( _poTempFont, "[ FTEXT ] Error: NULL pointer (invalid font handle) !!!" );
	//
	////

	return _poTempFont->fWordSpacing;

} // ftext_GetWordSpacing

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

f32 ftext_GetItalicSlant( const FDataFntFile_Handle_t ohFont )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );
	FASSERT_MSG( ohFont,            "[ FTEXT ] Error: NULL handle !!!" );

	////
	//
	_poTempFont = _poFontsHead;

	do
	{
		if( _poTempFont->oHandle == ohFont )
		{
			break;
		}

		_poTempFont = _poTempFont->poFontNext;

	} while( _poTempFont );

	FASSERT_MSG( _poTempFont, "[ FTEXT ] Error: NULL pointer (invalid font handle) !!!" );
	//
	////

	return _poTempFont->fItalicSlant;

} // ftext_GetItalicSlant

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_SetLetterSpacing( const FDataFntFile_Handle_t ohFont, const f32 fLetterSpacing )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );
	FASSERT_MSG( ohFont,            "[ FTEXT ] Error: NULL handle !!!" );

	////
	//
	_poTempFont = _poFontsHead;

	do
	{
		if( _poTempFont->oHandle == ohFont )
		{
			break;
		}

		_poTempFont = _poTempFont->poFontNext;

	} while( _poTempFont );

	FASSERT_MSG( _poTempFont, "[ FTEXT ] Error: NULL pointer (invalid font handle) !!!" );
	//
	////

	_poTempFont->fLetterSpacing = fLetterSpacing;

} // ftext_SetLetterSpacing

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_SetWordSpacing( const FDataFntFile_Handle_t ohFont, const f32 fWordSpacing )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );
	FASSERT_MSG( ohFont,            "[ FTEXT ] Error: NULL handle !!!" );

	////
	//
	_poTempFont = _poFontsHead;

	do
	{
		if( _poTempFont->oHandle == ohFont )
		{
			break;
		}

		_poTempFont = _poTempFont->poFontNext;

	} while( _poTempFont );

	FASSERT_MSG( _poTempFont, "[ FTEXT ] Error: NULL pointer (invalid font handle) !!!" );
	//
	////

	_poTempFont->fWordSpacing = fWordSpacing;

} // ftext_SetWordSpacing

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_SetItalicSlant( const FDataFntFile_Handle_t ohFont, const f32 fItalicSlant )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );
	FASSERT_MSG( ohFont,            "[ FTEXT ] Error: NULL handle !!!" );

	////
	//
	_poTempFont = _poFontsHead;

	do
	{
		if( _poTempFont->oHandle == ohFont )
		{
			break;
		}

		_poTempFont = _poTempFont->poFontNext;

	} while( _poTempFont );

	FASSERT_MSG( _poTempFont, "[ FTEXT ] Error: NULL pointer (invalid font handle) !!!" );
	//
	////

	_poTempFont->fItalicSlant = fItalicSlant;

} // ftext_SetItalicSlant

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_Printf( const FTextAreaHandle_t ohArea, cchar *pszFormat, ... )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	va_list	oVAList;
	va_start( oVAList, pszFormat );
	if( -1 == _vsnprintf( (char *)_pszTemp, Fang_ConfigDefs.nText_MaxCharsPerPrintf, pszFormat, oVAList ) )
	{
		FASSERT_NOW_MSG( "[ FTEXT ] Error: _vsnprintf() destination buffer too small !!!" );
	}
	va_end( oVAList );

	//now, convert the single byte string to a multibyte string
	fclib_CharToWide( _pszWTemp, _pszTemp );

	_ftext_PrintString( _FTEXT_PRINTF_AREA_ONLY, ohArea, 0.0f, 0.0f, _pszWTemp );

} // ftext_PrintfA

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_Printf( const FTextAreaHandle_t ohArea, cwchar *pszWFormat, ... )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	va_list	oVAList;
	va_start( oVAList, pszWFormat );
	if( -1 == _vsnwprintf( _pszWTemp, Fang_ConfigDefs.nText_MaxCharsPerPrintf, pszWFormat, oVAList ) )
	{
		FASSERT_NOW_MSG( "[ FTEXT ] Error: _vsnprintf() destination buffer too small !!!" );
	}
	va_end( oVAList );

	_ftext_PrintString( _FTEXT_PRINTF_AREA_ONLY, ohArea, 0.0f, 0.0f, _pszWTemp );

} // ftext_PrintfW

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_Printf( const FTextAreaHandle_t ohArea, const f32 fX, const f32 fY, cchar *pszFormat, ... )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	va_list	oVAList;
	va_start( oVAList, pszFormat );
	if( -1 == _vsnprintf( (char *)_pszTemp, Fang_ConfigDefs.nText_MaxCharsPerPrintf, pszFormat, oVAList ) )
	{
		FASSERT_NOW_MSG( "[ FTEXT ] Error: _vsnprintf() destination buffer too small !!!" );
	}
	va_end( oVAList );

	//now, convert the single byte string to a multibyte string
	fclib_CharToWide( _pszWTemp, _pszTemp );

	_ftext_PrintString( _FTEXT_PRINTF_AREA_AND_XY, ohArea, fX, fY, _pszWTemp );

} // ftext_PrintfA

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_Printf( const FTextAreaHandle_t ohArea, const f32 fX, const f32 fY, cwchar *pszWFormat, ... )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	va_list	oVAList;
	va_start( oVAList, pszWFormat );
	if( -1 == _vsnwprintf( _pszWTemp, Fang_ConfigDefs.nText_MaxCharsPerPrintf, pszWFormat, oVAList ) )
	{
		FASSERT_NOW_MSG( "[ FTEXT ] Error: _vsnprintf() destination buffer too small !!!" );
	}
	va_end( oVAList );

	_ftext_PrintString( _FTEXT_PRINTF_AREA_AND_XY, ohArea, fX, fY, _pszWTemp );

} // ftext_PrintfW

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_Printf( const f32 fX, const f32 fY, cchar *pszFormat, ... )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	va_list	oVAList;
	va_start( oVAList, pszFormat );
	if( -1 == _vsnprintf( (char *)_pszTemp, Fang_ConfigDefs.nText_MaxCharsPerPrintf, pszFormat, oVAList ) )
	{
		FASSERT_NOW_MSG( "[ FTEXT ] Error: _vsnprintf() destination buffer too small !!!" );
	}
	va_end( oVAList );

	//now, convert the single byte string to a multibyte string
	fclib_CharToWide( _pszWTemp, _pszTemp );

	_ftext_PrintString( _FTEXT_PRINTF_XY_ONLY, 0, fX, fY, _pszWTemp );

} // ftext_PrintfA

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_Printf( const f32 fX, const f32 fY, cwchar *pszWFormat, ... )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	va_list	oVAList;
	va_start( oVAList, pszWFormat );
	if( -1 == _vsnwprintf( _pszWTemp, Fang_ConfigDefs.nText_MaxCharsPerPrintf, pszWFormat, oVAList ) )
	{
		FASSERT_NOW_MSG( "[ FTEXT ] Error: _vsnprintf() destination buffer too small !!!" );
	}
	va_end( oVAList );

	fclib_WideToChar(_pszTemp, _pszWTemp);
	
	_ftext_PrintString( _FTEXT_PRINTF_XY_ONLY, 0, fX, fY, _pszWTemp );

} // ftext_PrintfW

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_DebugPrintf( const f32 fX, const f32 fY, cchar *pszFormat, ... )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	va_list	oVAList;
	va_start( oVAList, pszFormat );
	if( -1 == _vsnprintf( (char *)_pszTemp, Fang_ConfigDefs.nText_MaxCharsPerPrintf, pszFormat, oVAList ) )
	{
		FASSERT_NOW_MSG( "[ FTEXT ] Error: _vsnprintf() destination buffer too small !!!" );
	}
	va_end( oVAList );

	//now, convert the single byte string to a multibyte string
	fclib_CharToWide( _pszWTemp, _pszTemp );

	_ftext_PrintString( _FTEXT_PRINTF_DEBUG_XY_ONLY, 0, fX, fY, _pszWTemp );

} // ftext_DebugPrintfA

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_DebugPrintf( const f32 fX, const f32 fY, cwchar *pszWFormat, ... )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	va_list	oVAList;
	va_start( oVAList, pszWFormat );
	if( -1 == _vsnwprintf( _pszWTemp, Fang_ConfigDefs.nText_MaxCharsPerPrintf, pszWFormat, oVAList ) )
	{
		FASSERT_NOW_MSG( "[ FTEXT ] Error: _vsnprintf() destination buffer too small !!!" );
	}
	va_end( oVAList );

	_ftext_PrintString( _FTEXT_PRINTF_DEBUG_XY_ONLY, 0, fX, fY, _pszWTemp );

} // ftext_DebugPrintfW

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_PrintString( const FTextAreaHandle_t ohArea, cchar *pszString )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	//now, convert the single byte string to a multibyte string
	fclib_CharToWide( _pszWTemp, pszString );

	_ftext_PrintString( _FTEXT_PRINTF_AREA_ONLY, ohArea, 0.0f, 0.0f, _pszWTemp );

} // ftext_PrintStringA

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_PrintString( const FTextAreaHandle_t ohArea, cwchar *pszWString )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	_ftext_PrintString( _FTEXT_PRINTF_AREA_ONLY, ohArea, 0.0f, 0.0f, pszWString );

} // ftext_PrintStringW

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_PrintString( const FTextAreaHandle_t ohArea, const f32 fX, const f32 fY, cchar *pszString )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	//now, convert the single byte string to a multibyte string
	fclib_CharToWide( _pszWTemp, pszString );

	_ftext_PrintString( _FTEXT_PRINTF_AREA_AND_XY, ohArea, fX, fY, _pszWTemp );

} // ftext_PrintStringA

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_PrintString( const FTextAreaHandle_t ohArea, const f32 fX, const f32 fY, cwchar *pszWString )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	_ftext_PrintString( _FTEXT_PRINTF_AREA_AND_XY, ohArea, fX, fY, pszWString );

} // ftext_PrintStringW

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_PrintString( const f32 fX, const f32 fY, cchar *pszString )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	//now, convert the single byte string to a multibyte string
	fclib_CharToWide( _pszWTemp, pszString );

	_ftext_PrintString( _FTEXT_PRINTF_XY_ONLY, 0, fX, fY, _pszWTemp );

} // ftext_PrintStringA

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_PrintString( const f32 fX, const f32 fY, cwchar *pszWString )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	_ftext_PrintString( _FTEXT_PRINTF_XY_ONLY, 0, fX, fY, pszWString );

} // ftext_PrintStringW

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_DebugPrintString( const f32 fX, const f32 fY, cchar *pszString )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	//now, convert the single byte string to a multibyte string
	fclib_CharToWide( _pszWTemp, pszString );

	_ftext_PrintString( _FTEXT_PRINTF_DEBUG_XY_ONLY, 0, fX, fY, _pszWTemp );

} // ftext_DebugPrintStringA

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_DebugPrintString( const f32 fX, const f32 fY, cwchar *pszWString )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	_ftext_PrintString( _FTEXT_PRINTF_DEBUG_XY_ONLY, 0, fX, fY, pszWString );

} // ftext_DebugPrintString

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// fZ must be 0.0f or greater.
void ftext_Printf_ZPush( f32 fX, f32 fY, f32 fZ, cchar *pszFormat, ... ) {
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );
	FASSERT( fZ >= 0.0f );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	va_list	oVAList;
	va_start( oVAList, pszFormat );
	if( -1 == _vsnprintf( (char *)_pszTemp, Fang_ConfigDefs.nText_MaxCharsPerPrintf, pszFormat, oVAList ) )
	{
		FASSERT_NOW_MSG( "[ FTEXT ] Error: _vsnprintf() destination buffer too small !!!" );
	}
	va_end( oVAList );

	//now, convert the single byte string to a multibyte string
	fclib_CharToWide( _pszWTemp, _pszTemp );

	_ftext_PrintString( _FTEXT_PRINTF_XY_ONLY, 0, fX, fY, _pszWTemp, 1.0f + fZ );

}

// fZ must be 0.0f or greater.
void ftext_PrintString_ZPush( f32 fX, f32 fY, f32 fZ, cchar *pszString ) {
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );
	FASSERT( fZ >= 0.0f );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	//now, convert the single byte string to a multibyte string
	fclib_CharToWide( _pszWTemp, pszString );

	_ftext_PrintString( _FTEXT_PRINTF_XY_ONLY, 0, fX, fY, _pszWTemp, 1.0f + fZ );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _ftext_PrintString(  const _FTextPrintf_e oType,
							     const FTextAreaHandle_t ohArea,
								 const f32 fX,
								 const f32 fY,
								 cwchar *pszWString,
								 const f32 fZ/*=1.0f*/ ) {

	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );
	FASSERT_MSG( pszWString,         "[ FTEXT ] Error: NULL pointer !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	if( ! pszWString[ 0 ] )
	{
		return;
	}

	_InitializeWorkingVarsBasedOnPrintfType( oType, ohArea, fX, fY );

	_nLettersInCurrentLine = 0;
	_nLettersInCurrentWord = 0;

	//find the current font we are using for this area...
	_poTempFont2 = _poFontsHead;
	do
	{
		if( _poTempFont2->oHandle == _poTempArea->oAttributes.ohFont )
		{
			_poTempFont = _poTempFont2;
			break;
		}

		_poTempFont2 = _poTempFont2->poFontNext;

	} while( _poTempFont2 );

	//// Defaults.
	//
	if ( _poTempArea->oAttributes.bNoScale) {
		_fTempFrameBufferXRescaleFactor = 1.0f;
		_fTempFontScale = 1.0f;
		_bTempOTOTexelPixelAlignment = TRUE;
	}
	else {
		_fTempFrameBufferXRescaleFactor = _FRAMEBUFFER_X_SCALEFACTOR;
		_fTempFontScale = _fTempOriginalScale = ( ( _FTEXT_PRINTF_DEBUG_XY_ONLY == oType ) ? 1.0f : fmath_Div( ( fmath_Div( ( _poTempArea->oAttributes.fLowerRightY - ( _poTempArea->oAttributes.fUpperLeftY + ( 2 * _poTempArea->oAttributes.fBorderThicknessY ) + ( _poTempArea->oAttributes.fNumberOfLines * _poTempArea->oAttributes.fLineSpacing ) ) ), _poTempArea->oAttributes.fNumberOfLines ) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH * _fTempResY ), _poTempFont->fHeight ) );
		_bTempOTOTexelPixelAlignment = ( ( _FTEXT_PRINTF_DEBUG_XY_ONLY == oType ) ? TRUE : FALSE );
	}

	_fTempItalicSlantUnscaled = _poTempFont->fItalicSlant;
	_bTempItalicFont = _poTempArea->oAttributes.bItalicFont;
	_fTempItalicSlant = ( _bTempItalicFont ? ( _fTempItalicSlantUnscaled * _fTempFontScale ) : 0.0f );


	_bTempFixedWidthFont = _poTempArea->oAttributes.bFixedWidthFont;
	_fTempSpaceCharSize = ( _bTempFixedWidthFont ? ( ( _poTempFont->fWidth + _poTempFont->fLetterSpacing ) * _fTempFontScale ) : ( _poTempFont->fWordSpacing * _fTempFontScale ) );

	_fTempVertOffset = ( _fTempResY * _poTempArea->oAttributes.fLineSpacing * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH * 0.5f );
	_fTempBlinkingAlpha = 1.0f;
	_fTempBaseAlpha = _fTempGeneralAlpha = _poTempArea->oAttributes.oColorForeground.fAlpha * _poTempArea->oAttributes.fAlpha;
	_oTempColor = _poTempArea->oAttributes.oColorForeground;
	_oTempColor.fAlpha = _fTempGeneralAlpha;

	_uIndex = 0; // Char index.
	//
	////

	//parse the text string
	do
	{
		//// Escape Code character sequence.
		//
		if( L'~' == pszWString[ _uIndex ] ) //escape code
		{
			if( _ParseEscapeCode( oType, fX, pszWString ) ) {
				continue;
			}
		}
		//
		////

		FASSERT_MSG( _poTempFont, "[ FTEXT ] Error: NULL pointer (No current font) !!!" );

		//// Space.
		//
		if( L' ' == pszWString[ _uIndex ] ) //deal with a space
		{
			_fTempX += _fTempSpaceCharSize * _fTempFrameBufferXRescaleFactor;
			_nLettersInCurrentWord = 0;
			continue;
		}
		//
		////

		//// Tab.
		//
		if( L'\t' == pszWString[ _uIndex ] )
		{
			_fTempX += ( _fTempSpaceCharSize * _poTempArea->oAttributes.fTabSize * _fTempFrameBufferXRescaleFactor );
			_nLettersInCurrentWord = 0;
			continue;
		}
		//
		////

		//// New line.
		//
		if( L'\n' == pszWString[ _uIndex ] )
		{
			if( FTEXT_HORZ_ALIGN_LEFT != _oTempHorzAlign )
			{
				//calculate the alignment issues for the current line before we move to the next line
				_CalculateHorizontalAlignmentOffset( oType, fX ); 
			}
			_fTempX = _fTempCursorOriginX;
			_fTempY += ( ( _poTempFont->fHeight * _fTempFontScale ) + ( _fTempResY *_poTempArea->oAttributes.fLineSpacing * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH ) );

			_nLettersInCurrentWord = 0;
			_nLettersInCurrentLine = 0;
			continue;
		}
		//
		////

		// Culling.
		if( 1.0f >= ( _fTempLowerRightInnerEdgeY - _fTempY ) )
		{
			break;
		}

		//// Letter.
		//
		if ( !_ParseLetter( oType, fX, pszWString, fZ) ) {
			break;
		}
		//
		////

	} while( pszWString[ ++_uIndex ] );


	//// Horizontal alignment (align on EOS).
	//
	if( FTEXT_HORZ_ALIGN_LEFT != _oTempHorzAlign )
	{
		//if we are in here, then the string either has center or
		//rightmost alignment.  Calculate the new alignment offset and run through
		//the existing vertices and apply the offset.
		_CalculateHorizontalAlignmentOffset( oType, fX );
	}
	//
	//// Horizontal alignment (align on EOS).

	if( _FTEXT_PRINTF_AREA_ONLY == oType )
	{
		_fTempY += ( ( _poTempFont->fHeight * _fTempFontScale ) + ( _fTempResY *_poTempArea->oAttributes.fLineSpacing * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH ) );
		_poTempArea->fUnitCursorY = fmath_Div( ( fmath_Div( _fTempY, ( _fTempResY * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH ) ) - ( _poTempArea->oAttributes.fUpperLeftY + _poTempArea->oAttributes.fBorderThicknessY ) ), ( ( _poTempArea->oAttributes.fLowerRightY - _poTempArea->oAttributes.fBorderThicknessY ) - ( _poTempArea->oAttributes.fUpperLeftY + _poTempArea->oAttributes.fBorderThicknessY ) ) );
	}
} // _ftext_PrintString2

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//This function will set up the printf working variables based on the type
//of printf we want to do.
void _InitializeWorkingVarsBasedOnPrintfType( const _FTextPrintf_e oType,
											  const FTextAreaHandle_t ohArea,
 											  const f32 fX,
											  const f32 fY ) {

	FViewport_t *pVP = fviewport_GetActive();
	if (pVP == NULL) {
		pVP = FViewport_pDefaultOrtho;
	}

	float fVpLeft = pVP->ScreenCorners.m_UpperLeft.x * _FRAMEBUFFER_X_SCALEFACTOR;
	float fVpTop  = pVP->ScreenCorners.m_UpperLeft.y;

	if( _FTEXT_PRINTF_AREA_ONLY == oType )
	{
		_poTempArea = ( ohArea ? (_FTextArea_t *)ohArea : _poAreasHead );

		// Save the viewport boundaries for later border drawing
		_poTempArea->oVpRect = pVP->ScreenCorners;
		_poTempArea->oVpRes = pVP->Res;
		
		//now, fix up the horizontal resolution with the frame buffer x rescale factor
		_poTempArea->oVpRect.m_UpperLeft.x *= _FRAMEBUFFER_X_SCALEFACTOR;
		_poTempArea->oVpRect.m_LowerRight.x *= _FRAMEBUFFER_X_SCALEFACTOR;
		_poTempArea->oVpRes.x *= _FRAMEBUFFER_X_SCALEFACTOR;
		
		_fTempResX = pVP->Res.x * _FRAMEBUFFER_X_SCALEFACTOR;
		_fTempResY = pVP->Res.y;

		// XY area cursor.
		_fTempX = _fTempCursorOriginX = fVpLeft + ( _fTempResX * ( _poTempArea->oAttributes.fUpperLeftX + _poTempArea->oAttributes.fBorderThicknessX * _FRAMEBUFFER_X_SCALEFACTOR ) );
		_fTempY = _fTempCursorOriginY = fVpTop  + ( _fTempResY * ( ( _poTempArea->oAttributes.fUpperLeftY + _poTempArea->oAttributes.fBorderThicknessY ) + ( _poTempArea->fUnitCursorY * ( ( _poTempArea->oAttributes.fLowerRightY - _poTempArea->oAttributes.fBorderThicknessY ) - ( _poTempArea->oAttributes.fUpperLeftY + _poTempArea->oAttributes.fBorderThicknessY ) ) ) ) ) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH;

		_oTempHorzAlign = _poTempArea->oAttributes.oHorzAlign;

		_fTempLowerRightInnerEdgeX = fVpLeft + ( _fTempResX * ( _poTempArea->oAttributes.fLowerRightX - _poTempArea->oAttributes.fBorderThicknessX * _FRAMEBUFFER_X_SCALEFACTOR ) );
	}
	else if( _FTEXT_PRINTF_AREA_AND_XY == oType )
	{
		_poTempArea = ( ohArea ? (_FTextArea_t *)ohArea : _poAreasHead );

		// Save the viewport boundaries for later border drawing
		_poTempArea->oVpRect = pVP->ScreenCorners;
		_poTempArea->oVpRes = pVP->Res;

		//now, fix up the horizontal resolution with the frame buffer x rescale factor
		_poTempArea->oVpRect.m_UpperLeft.x *= _FRAMEBUFFER_X_SCALEFACTOR;
		_poTempArea->oVpRect.m_LowerRight.x *= _FRAMEBUFFER_X_SCALEFACTOR;
		_poTempArea->oVpRes.x *= _FRAMEBUFFER_X_SCALEFACTOR;
		
		_fTempResX = pVP->Res.x * _FRAMEBUFFER_X_SCALEFACTOR;
		_fTempResY = pVP->Res.y;

		// XY area cursor.
		_fTempX = _fTempCursorOriginX = fVpLeft + ( _fTempResX * ( _poTempArea->oAttributes.fUpperLeftX + _poTempArea->oAttributes.fBorderThicknessX * _FRAMEBUFFER_X_SCALEFACTOR + ( fX * ( _poTempArea->oAttributes.fLowerRightX - _poTempArea->oAttributes.fUpperLeftX - ( 2.0f * _poTempArea->oAttributes.fBorderThicknessX * _FRAMEBUFFER_X_SCALEFACTOR ) ) ) ) );
		_fTempY = _fTempCursorOriginY = fVpTop + ( _fTempResY * ( _poTempArea->oAttributes.fUpperLeftY + _poTempArea->oAttributes.fBorderThicknessY + ( fY * ( _poTempArea->oAttributes.fLowerRightY - _poTempArea->oAttributes.fUpperLeftY - ( 2.0f * _poTempArea->oAttributes.fBorderThicknessY ) ) ) ) ) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH;

		_oTempHorzAlign = _poTempArea->oAttributes.oHorzAlign;

		if( FTEXT_HORZ_ALIGN_LEFT == _oTempHorzAlign )
		{
			_fTempLowerRightInnerEdgeX = ( _fTempResX * ( _poTempArea->oAttributes.fLowerRightX - _poTempArea->oAttributes.fBorderThicknessX * _FRAMEBUFFER_X_SCALEFACTOR ) );
		}
		else if( FTEXT_HORZ_ALIGN_CENTER == _oTempHorzAlign )
		{
			_fTempLowerRightInnerEdgeX = ( ( fX <= 0.5f ) ? ( _fTempCursorOriginX + ( 2.0f * ( _fTempCursorOriginX - ( _fTempResX * ( _poTempArea->oAttributes.fUpperLeftX + _poTempArea->oAttributes.fBorderThicknessX * _FRAMEBUFFER_X_SCALEFACTOR ) ) ) ) ) : ( _fTempCursorOriginX + ( 2.0f * ( ( _fTempResX * ( _poTempArea->oAttributes.fLowerRightX - _poTempArea->oAttributes.fBorderThicknessX * _FRAMEBUFFER_X_SCALEFACTOR ) ) - _fTempCursorOriginX ) ) ) );
		}
		else // FTEXT_HORZ_ALIGN_RIGHT
		{
			_fTempLowerRightInnerEdgeX = ( _fTempCursorOriginX + _fTempCursorOriginX - ( _fTempResX * ( _poTempArea->oAttributes.fUpperLeftX + _poTempArea->oAttributes.fBorderThicknessX * _FRAMEBUFFER_X_SCALEFACTOR ) ) );
		}
		_fTempLowerRightInnerEdgeX += fVpLeft;
	}
	else // _FTEXT_PRINTF_XY_ONLY || _FTEXT_PRINTF_DEBUG_XY_ONLY
	{
		_poTempArea = _poAreasHead;

		// Save the viewport boundaries for later border drawing
		_poTempArea->oVpRect = pVP->ScreenCorners;
		_poTempArea->oVpRes = pVP->Res;

		//now, fix up the horizontal resolution with the frame buffer x rescale factor
		_poTempArea->oVpRect.m_UpperLeft.x *= _FRAMEBUFFER_X_SCALEFACTOR;
		_poTempArea->oVpRect.m_LowerRight.x *= _FRAMEBUFFER_X_SCALEFACTOR;
		_poTempArea->oVpRes.x *= _FRAMEBUFFER_X_SCALEFACTOR;
		
		_fTempResX = pVP->Res.x * _FRAMEBUFFER_X_SCALEFACTOR;
		_fTempResY = pVP->Res.y;

		// XY area cursor.
		_fTempX = _fTempCursorOriginX = fVpLeft + ( _fTempResX * fX );
		_fTempY = _fTempCursorOriginY = fVpTop + ( _fTempResY * fY ) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH;

		_oTempHorzAlign = _poTempArea->oAttributes.oHorzAlign;

		if( FTEXT_HORZ_ALIGN_LEFT == _oTempHorzAlign )
		{
			_fTempLowerRightInnerEdgeX = _fTempResX;
		}
		else if( FTEXT_HORZ_ALIGN_CENTER == _oTempHorzAlign )
		{
			_fTempLowerRightInnerEdgeX = ( ( fX <= 0.5f ) ? ( _fTempCursorOriginX + _fTempCursorOriginX + _fTempCursorOriginX ) : ( _fTempCursorOriginX + ( 2.0f * ( _fTempResX * ( 1.0f - fX ) ) ) ) );
		}
		else // FTEXT_HORZ_ALIGN_RIGHT
		{
			_fTempLowerRightInnerEdgeX = ( _fTempCursorOriginX + _fTempCursorOriginX );
		}
		_fTempLowerRightInnerEdgeX += fVpLeft;
	}
	_fTempLowerRightInnerEdgeY = fVpTop + ( ( _fTempResY * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH ) * ( _poTempArea->oAttributes.fLowerRightY - _poTempArea->oAttributes.fBorderThicknessY ) );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//This function returns a true or false value as to wheter the escape code was successfully parsed
BOOL _ParseEscapeCode ( const _FTextPrintf_e oType, const f32 fX, cwchar *pszWString ) {
	_uTemp = _uIndex++;

	if( ( L'a' == pszWString[ _uIndex ] ) || ( L'A' == pszWString[ _uIndex ] ) )
	{
		++_uIndex;
		// Set current alignment.
		if( ( L'l' == pszWString[ _uIndex ] ) || ( L'L' == pszWString[ _uIndex ] ) )
		{
			_oTempHorzAlign = FTEXT_HORZ_ALIGN_LEFT;

			if( _FTEXT_PRINTF_AREA_AND_XY == oType )
			{
				_fTempLowerRightInnerEdgeX = _poTempArea->oVpRect.m_UpperLeft.x + ( _fTempResX * ( _poTempArea->oAttributes.fLowerRightX - _poTempArea->oAttributes.fBorderThicknessX ) );
			}
			else if( ( _FTEXT_PRINTF_XY_ONLY == oType ) || ( _FTEXT_PRINTF_DEBUG_XY_ONLY == oType ) )
			{
				_fTempLowerRightInnerEdgeX = _poTempArea->oVpRect.m_LowerRight.x;
			}

			return TRUE;
		}
		else if( ( L'c' == pszWString[ _uIndex ] ) || ( L'C' == pszWString[ _uIndex ] ) )
		{
			_oTempHorzAlign = FTEXT_HORZ_ALIGN_CENTER;

			if( _FTEXT_PRINTF_AREA_AND_XY == oType )
			{
				_fTempLowerRightInnerEdgeX = ( ( fX <= 0.5f ) ? ( _fTempCursorOriginX + ( 2.0f * ( _fTempCursorOriginX - ( _fTempResX * ( _poTempArea->oAttributes.fUpperLeftX + _poTempArea->oAttributes.fBorderThicknessX ) ) ) ) ) : ( _fTempCursorOriginX + ( 2.0f * ( ( _fTempResX * ( _poTempArea->oAttributes.fLowerRightX - _poTempArea->oAttributes.fBorderThicknessX ) ) - _fTempCursorOriginX ) ) ) );
			}
			else if( ( _FTEXT_PRINTF_XY_ONLY == oType ) || ( _FTEXT_PRINTF_DEBUG_XY_ONLY == oType ) )
			{
				_fTempLowerRightInnerEdgeX = ( ( fX <= 0.5f ) ? ( _fTempCursorOriginX + _fTempCursorOriginX + _fTempCursorOriginX ) : ( _fTempCursorOriginX + ( 2.0f * ( _fTempResX * ( 1.0f - fX ) ) ) ) );
			}

			return TRUE;
		}
		else if( ( L'r' == pszWString[ _uIndex ] ) || ( L'R' == pszWString[ _uIndex ] ) )
		{
			_oTempHorzAlign = FTEXT_HORZ_ALIGN_RIGHT;

			if( _FTEXT_PRINTF_AREA_AND_XY == oType )
			{
				_fTempLowerRightInnerEdgeX = ( _fTempCursorOriginX + _fTempCursorOriginX - ( _fTempResX * ( _poTempArea->oAttributes.fUpperLeftX + _poTempArea->oAttributes.fBorderThicknessX ) ) );
			}
			else if( ( _FTEXT_PRINTF_XY_ONLY == oType ) || ( _FTEXT_PRINTF_DEBUG_XY_ONLY == oType ) )
			{
				_fTempLowerRightInnerEdgeX = ( _fTempCursorOriginX + _fTempCursorOriginX );
			}

			return TRUE;
		}
		else
		{
			_uIndex = _uTemp;
		}
	}
	else if( ( L'b' == pszWString[ _uIndex ] ) || ( L'B' == pszWString[ _uIndex ] ) )
	{
		++_uIndex;
		// Set current blinking.
		if( ( L'0' <= pszWString[ _uIndex ] ) && ( L'9' >= pszWString[ _uIndex ] ) )
		{
			if( _bBlinkingAlphasNeedUpdate )
			{
				_bBlinkingAlphasNeedUpdate = FALSE;
				_ftext_UpdateBlinkingAlphas();
			}

			_fTempBlinkingAlpha = _aoBlinkingAlphas[ pszWString[ _uIndex ] - L'0' ].fAlpha;
			_oTempColor.fAlpha = _fTempBaseAlpha * _fTempGeneralAlpha * _fTempBlinkingAlpha;

			return TRUE;
		}
		else if( ( L'a' == pszWString[ _uIndex ] ) || ( L'A' == pszWString[ _uIndex ] ) )
		{
			if( _bBlinkingAlphasNeedUpdate )
			{
				_bBlinkingAlphasNeedUpdate = FALSE;
				_ftext_UpdateBlinkingAlphas();
			}

			_fTempBlinkingAlpha = _aoBlinkingAlphas[ 10 ].fAlpha;
			_oTempColor.fAlpha = _fTempBaseAlpha * _fTempGeneralAlpha * _fTempBlinkingAlpha;

			return TRUE;
		}
		else
		{
			_uIndex = _uTemp;
		}
	}
	else if( ( L'c' == pszWString[ _uIndex ] ) || ( L'C' == pszWString[ _uIndex ] ) )
	{
		// Set current color.
		++_uIndex;
		if( ( ( L'0' <= pszWString[ _uIndex + 0 ] ) && ( L'9' >= pszWString[ _uIndex + 0 ] ) ) && // R
			( ( L'0' <= pszWString[ _uIndex + 1 ] ) && ( L'9' >= pszWString[ _uIndex + 1 ] ) ) && // R
			( ( L'0' <= pszWString[ _uIndex + 2 ] ) && ( L'9' >= pszWString[ _uIndex + 2 ] ) ) && // G
			( ( L'0' <= pszWString[ _uIndex + 3 ] ) && ( L'9' >= pszWString[ _uIndex + 3 ] ) ) && // G
			( ( L'0' <= pszWString[ _uIndex + 4 ] ) && ( L'9' >= pszWString[ _uIndex + 4 ] ) ) && // B
			( ( L'0' <= pszWString[ _uIndex + 5 ] ) && ( L'9' >= pszWString[ _uIndex + 5 ] ) ) && // B
			( ( L'0' <= pszWString[ _uIndex + 6 ] ) && ( L'9' >= pszWString[ _uIndex + 6 ] ) ) && // A
			( ( L'0' <= pszWString[ _uIndex + 7 ] ) && ( L'9' >= pszWString[ _uIndex + 7 ] ) ) )  // A
		{
			_fTempBaseAlpha = ( (f32)( ( ( pszWString[ _uIndex + 6 ] - L'0' ) * 10 ) + ( pszWString[ _uIndex + 7 ] - L'0' ) ) * ( 1.0f / 99.0f ) );
			_oTempColor.Set( ( (f32)( ( ( pszWString[ _uIndex + 0 ] - L'0' ) * 10 ) + ( pszWString[ _uIndex + 1 ] - L'0' ) ) * ( 1.0f / 99.0f ) ),
								( (f32)( ( ( pszWString[ _uIndex + 2 ] - L'0' ) * 10 ) + ( pszWString[ _uIndex + 3 ] - L'0' ) ) * ( 1.0f / 99.0f ) ),
								( (f32)( ( ( pszWString[ _uIndex + 4 ] - L'0' ) * 10 ) + ( pszWString[ _uIndex + 5 ] - L'0' ) ) * ( 1.0f / 99.0f ) ),
								( _fTempBaseAlpha * _fTempGeneralAlpha * _fTempBlinkingAlpha ) );
			_uIndex += 7;
			return TRUE;
		}
		else
		{
			_uIndex = _uTemp;
		}
	}
	else if( ( L'f' == pszWString[ _uIndex ] ) || ( L'F' == pszWString[ _uIndex ] ) )
	{
		++_uIndex;
		if( pszWString[ _uIndex ] )
		{
			// Set current font.
			_poTempFont2 = _poFontsHead;

			do
			{
				if( _poTempFont2->oHandle == pszWString[ _uIndex ] )
				{
					break;
				}

				_poTempFont2 = _poTempFont2->poFontNext;

			} while( _poTempFont2 );

			if( _poTempFont2 )
			{
				_poTempFont = _poTempFont2;

				_fTempFontScale = _fTempOriginalScale = ( ( ( _FTEXT_PRINTF_DEBUG_XY_ONLY == oType ) || _bTempOTOTexelPixelAlignment ) ? 1.0f : fmath_Div( ( fmath_Div( ( _poTempArea->oAttributes.fLowerRightY - ( _poTempArea->oAttributes.fUpperLeftY + ( 2 * _poTempArea->oAttributes.fBorderThicknessY ) + ( _poTempArea->oAttributes.fNumberOfLines * _poTempArea->oAttributes.fLineSpacing ) ) ), _poTempArea->oAttributes.fNumberOfLines ) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH * _fTempResY ), _poTempFont->fHeight ) );
				_fTempSpaceCharSize = ( _bTempFixedWidthFont ? ( ( _poTempFont->fWidth + _poTempFont->fLetterSpacing ) * _fTempFontScale ) : ( _poTempFont->fWordSpacing * _fTempFontScale ) );
				_fTempItalicSlantUnscaled = _poTempFont->fItalicSlant;
				_fTempItalicSlant = ( _bTempItalicFont ? ( _fTempItalicSlantUnscaled * _fTempFontScale ) : 0.0f );

				_fTempVertOffset = ( _fTempResY * _poTempArea->oAttributes.fLineSpacing * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH * 0.5f );
				return TRUE;
			}
			else
			{
				_uIndex = _uTemp;
			}
		}
		else
		{
			_uIndex = _uTemp;
		}
	}
	else if( ( L's' == pszWString[ _uIndex ] ) || ( L'S' == pszWString[ _uIndex ] ) )
	{
		// Set current scale.
		++_uIndex;
		if( ( ( L'0' <= pszWString[ _uIndex + 0 ] ) && ( L'9' >= pszWString[ _uIndex + 0 ] ) ) &&
				( L'.' == pszWString[ _uIndex + 1 ] ) &&
			( ( L'0' <= pszWString[ _uIndex + 2 ] ) && ( L'9' >= pszWString[ _uIndex + 2 ] ) ) &&
			( ( L'0' <= pszWString[ _uIndex + 3 ] ) && ( L'9' >= pszWString[ _uIndex + 3 ] ) ) )
		{
			_fTempFontScale = _fTempOriginalScale * ( (f32)( pszWString[ _uIndex + 0 ] - L'0' ) + ( (f32)( pszWString[ _uIndex + 2 ] - L'0' ) * ( 1.0f / 10.0f ) ) + ( (f32)( pszWString[ _uIndex + 3 ] - L'0' ) * ( 1.0f / 100.0f ) ) );
			_fTempVertOffset = ( 0.5f * ( ( _fTempResY * _poTempArea->oAttributes.fLineSpacing * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH ) - ( ( _fTempFontScale - _fTempOriginalScale ) * _poTempFont->fHeight ) ) );
			_fTempSpaceCharSize = ( _bTempFixedWidthFont ? ( ( _poTempFont->fWidth + _poTempFont->fLetterSpacing ) * _fTempFontScale ) : ( _poTempFont->fWordSpacing * _fTempFontScale ) );
			_fTempItalicSlant = ( _bTempItalicFont ? ( _fTempItalicSlantUnscaled * _fTempFontScale ) : 0.0f );

			_uIndex += 3;
			return TRUE;
		}
		else
		{
			_uIndex = _uTemp;
		}
	}
	else if( ( L't' == pszWString[ _uIndex ] ) || ( L'T' == pszWString[ _uIndex ] ) )
	{
		// Set current italic slant ("tilt").
		++_uIndex;
		if( ( ! ( ( L'+' != pszWString[ _uIndex + 0 ] ) && ( L'-' != pszWString[ _uIndex + 0 ] ) ) ) &&
			( ( L'0' <= pszWString[ _uIndex + 1 ] ) && ( L'9' >= pszWString[ _uIndex + 1 ] ) ) &&
			( ( L'0' <= pszWString[ _uIndex + 2 ] ) && ( L'9' >= pszWString[ _uIndex + 2 ] ) ) )
		{
			_fTempItalicSlantUnscaled = (f32)( ( ( pszWString[ _uIndex + 1 ] - L'0' ) * 10 ) + ( pszWString[ _uIndex + 2 ] - L'0' ) );
			if( L'-' == pszWString[ _uIndex + 0 ] )
			{
				_fTempItalicSlantUnscaled *= -1.0f;
			}
			if( _bTempItalicFont )
			{
				_fTempItalicSlant = ( _fTempItalicSlantUnscaled * _fTempFontScale );
			}

			_uIndex += 2;
			return TRUE;
		}
		else
		{
			_uIndex = _uTemp;
		}
	}
	else if( ( L'w' == pszWString[ _uIndex ] ) || ( L'W' == pszWString[ _uIndex ] ) )
	{
		++_uIndex;
		if( L'0' == pszWString[ _uIndex ] )
		{
			// Set current font to variable-width.
			_bTempFixedWidthFont = FALSE;
			_fTempSpaceCharSize = ( _poTempFont->fWordSpacing * _fTempFontScale );
			return TRUE;
		}
		else if( L'1' == pszWString[ _uIndex ] )
		{
			// Set current font to fixed-width.
			_bTempFixedWidthFont = TRUE;
			_fTempSpaceCharSize = ( ( _poTempFont->fWidth + _poTempFont->fLetterSpacing ) * _fTempFontScale );
			return TRUE;
		}
		else
		{
			_uIndex = _uTemp;
		}
	}
	else if( ( L'i' == pszWString[ _uIndex ] ) || ( L'I' == pszWString[ _uIndex ] ) )
	{
		++_uIndex;
		if( L'0' == pszWString[ _uIndex ] )
		{
			// Set current font to non italic.
			_bTempItalicFont = FALSE;
			_fTempItalicSlant = 0.0f;
			return TRUE;
		}
		else if( L'1' == pszWString[ _uIndex ] )
		{
			// Set current font to italic.
			_bTempItalicFont = TRUE;
			_fTempItalicSlant = ( _fTempItalicSlantUnscaled * _fTempFontScale );
			return TRUE;
		}
		else
		{
			_uIndex = _uTemp;
		}
	}
	else if( ( L'o' == pszWString[ _uIndex ] ) || ( L'O' == pszWString[ _uIndex ] ) )
	{
		//enable/disable one to one texel alignment
		++_uIndex;
		if( L'0' == pszWString[ _uIndex ] )
		{
			// Set current font to 'non one to one aligned'.
			_bTempOTOTexelPixelAlignment = FALSE;
			_fTempFrameBufferXRescaleFactor = _FRAMEBUFFER_X_SCALEFACTOR;
			return TRUE;
		}
		else if( L'1' == pszWString[ _uIndex ] )
		{
			// Set current font to 'one to one aligned'.
			_bTempOTOTexelPixelAlignment = TRUE;
			_fTempFrameBufferXRescaleFactor = 1.0f;
			_fTempFontScale = 1.0f;
			return TRUE;
		}
		else
		{
			_uIndex = _uTemp;
		}
	}
	else if( L'~' != pszWString[ _uIndex ] )
	{
		_uIndex = _uTemp;
	}

	return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//This function parses/processes the next letter in a text string to be printed.
//Return Values:
//	TRUE - Indicates everything is OK and Parsing of Text String should continue.
//	FALSE - Indicates error of some sort occured and parsing of rest of text string should be HALTED.
BOOL _ParseLetter( const _FTextPrintf_e oType, const f32 fX, cwchar *pszWString, const f32 fZ ) {

	//check to see if we have a free letter available
	if( _nLettersUsed >= Fang_ConfigDefs.nText_MaxCharsPerFrame ) {
		if( _uTextBufferOverflowErrors ) {
			DEVPRINTF( "[ FTEXT ] Warning %u: Max characters per Draw overflow (Max: %u chars). Skipping rest of text !!!\n", __LINE__, Fang_ConfigDefs.nText_MaxCharsPerFrame );
			_uTextBufferOverflowErrors--;
			if( ! _uTextBufferOverflowErrors )
			{
				DEVPRINTF( "[ FTEXT ] Warning %u: The previous warning will not be issued anymore !!!\n", __LINE__ );
			}
		}
		return FALSE;
	}

	fang_MemSet( &FText_paoPrintfLetters[ _nLettersUsed ], 0, sizeof ( FText_FTextPrintfLetter_t ) );
	u16 uFntLetterIndex = _FindFntLetterIndex( _poTempFont, pszWString[ _uIndex ] );
	_poTempLetter = NULL;
	if( uFntLetterIndex != 0xFFFF ){ //INVALID INDEX
		_poTempLetter = &_poTempFont->paoFntLetters[ uFntLetterIndex ];
	}

	if( ! _poTempLetter ) {
		// Try and find the null character from this font...
		uFntLetterIndex = _FindFntLetterIndex( _poTempFont, '`' );
		if( uFntLetterIndex != 0xFFFF ){ //INVALID INDEX
			_poTempLetter = &_poTempFont->paoFntLetters[ uFntLetterIndex ];
		}
	}

	FText_paoPrintfLetters[ _nLettersUsed ].hFont= _poTempFont->oHandle;
	FText_paoPrintfLetters[ _nLettersUsed ].uFntLetterIdx = uFntLetterIndex;
	FText_paoPrintfLetters[ _nLettersUsed ].fScale = _fTempFontScale;
	FText_paoPrintfLetters[ _nLettersUsed ].fItalicSlant = _fTempItalicSlant;

	if( _bTempFixedWidthFont ) {
		FText_paoPrintfLetters[ _nLettersUsed ].nStats |= _FTEXT_PRINTF_LETTERSTATS_FIXEDWIDTH;
	}
	if( _bTempOTOTexelPixelAlignment ) {
		FText_paoPrintfLetters[ _nLettersUsed ].nStats |= _FTEXT_PRINTF_LETTERSTATS_OTOTEXELPIXELALIGNMENT;
	}
	if( _FTEXT_PRINTF_DEBUG_XY_ONLY == oType ) {
		FText_paoPrintfLetters[ _nLettersUsed ].nStats |= _FTEXT_PRINTF_LETTERSTATS_DEBUG;
	}

#if 1
	if( ! _poTempLetter )
	{
		//go with the default set null character for right now....
		//RAFHACK -- THIS NEEDS TO BE CHANGED SOON!
		uFntLetterIndex = _FindFntLetterIndex( _poFontsHead, '`' ); // Should be _poTempFont->apoLetters[ 0 ].
		if( uFntLetterIndex != 0xFFFF ){ //INVALID INDEX
			_poTempLetter = &_poFontsHead->paoFntLetters[ uFntLetterIndex ];
		}
		FASSERT( _poTempLetter != NULL );

		FText_paoPrintfLetters[ _nLettersUsed ].hFont= _poFontsHead->oHandle;
		FText_paoPrintfLetters[ _nLettersUsed ].uFntLetterIdx = uFntLetterIndex;
        FText_paoPrintfLetters[ _nLettersUsed ].fX = _fTempX;
		FText_paoPrintfLetters[ _nLettersUsed ].fY = _fTempY + _fTempVertOffset;
		FText_paoPrintfLetters[ _nLettersUsed ].oColor.nABGR = 0xFFFFFFFF; //OpaqueWhite

		_fTempX += _fTempSpaceCharSize * _fTempFrameBufferXRescaleFactor;

		_poFontsHead->pauTextLengthPerTexturePages[ 0 ]++;
		_nLettersUsed++;
		_nLettersInCurrentWord++;
		_nLettersInCurrentLine++;

		return TRUE;
	}
#endif

	_uTemp = _poTempLetter->uTexurePage;

	//// Position letter (pre italicize scaling).
	//
	if( _bTempFixedWidthFont ) {
		// Upper Left
		_fTempFixupX1 = _fTempX + ( _poTempLetter->fHorzAlign * _fTempFontScale * _fTempFrameBufferXRescaleFactor );
		_fTempFixupY1 = _fTempY + _fTempVertOffset + ( _poTempLetter->fVertAlign * _fTempFontScale );
	} else {
		// Upper Left
		_fTempFixupX1 = _fTempX;
		_fTempFixupY1 = _fTempY + _fTempVertOffset + ( _poTempLetter->fVertAlign * _fTempFontScale );
	}

	//// Align texels to pixels.
	//
	if( ( _FTEXT_PRINTF_DEBUG_XY_ONLY == oType ) || _bTempOTOTexelPixelAlignment ) {
		_fTempFixupX1 = ( ( ( _fTempFixupX1 - (f32)( (s32)( _fTempFixupX1 ) ) ) > 0.5f ) ? ( (f32)( (s32)( _fTempFixupX1 ) ) + 1.5f ) : ( (f32)( (s32)( _fTempFixupX1 ) ) + 0.5f ) );
		_fTempFixupY1 = ( ( ( _fTempFixupY1 - (f32)( (s32)( _fTempFixupY1 ) ) ) > 0.5f ) ? ( (f32)( (s32)( _fTempFixupY1 ) ) + 1.5f ) : ( (f32)( (s32)( _fTempFixupY1 ) ) + 0.5f ) );
	}
	//
	////

    FText_paoPrintfLetters[ _nLettersUsed ].fX = _fTempFixupX1;
	FText_paoPrintfLetters[ _nLettersUsed ].fY = _fTempFixupY1;
	FText_paoPrintfLetters[ _nLettersUsed ].oColor.nRed   = (u8) (_oTempColor.fRed * 255.0f);
	FText_paoPrintfLetters[ _nLettersUsed ].oColor.nGreen = (u8) (_oTempColor.fGreen * 255.0f);
	FText_paoPrintfLetters[ _nLettersUsed ].oColor.nBlue  = (u8) (_oTempColor.fBlue * 255.0f);
	FText_paoPrintfLetters[ _nLettersUsed ].oColor.nAlpha = (u8) (_oTempColor.fAlpha * 255.0f);

	//
	//// Position letter.

	//increment the counter on the texture page indicator
	_uTemp = _poTempLetter->uTexurePage;
	_poTempFont->pauTextLengthPerTexturePages[ _uTemp ]++;

	_nLettersUsed++;
	_nLettersInCurrentWord++;
	_nLettersInCurrentLine++;

	//AT THIS POINT, THE CHARACTER PROCESSING IS ALL DONE -- NOW, WE NEED TO CHECK FOR WORD WRAP ISSUES
	//WHEN DEALING WITH THE CHARACTER, IT's IMPORTANT TO REMEBER THAT THAT STATS HAVE BEEN INCREMENTED
	//SO USE A -1 WHEN REFERENCING THE CURRENT CHARACTER

	f32 fUpperRightX, fLowerRightX;
	ftext_CalculateLetterCoordinates( &FText_paoPrintfLetters[ _nLettersUsed - 1 ], NULL, &fUpperRightX, NULL, &fLowerRightX, NULL, NULL);

	if( _bTempFixedWidthFont ) {
		//THIS CHECKS BOTH RIGHT TOP AND BOTTOM TO MAKE SURE IT FALLS WITHIN THE X BOUNDARIES
		if( ( ( fLowerRightX + ( _poTempLetter->fHorzAlign * _fTempFontScale * _fTempFrameBufferXRescaleFactor ) ) < _fTempLowerRightInnerEdgeX ) &&
			( ( fUpperRightX + ( _poTempLetter->fHorzAlign * _fTempFontScale * _fTempFrameBufferXRescaleFactor ) ) < _fTempLowerRightInnerEdgeX ) ) {
			// Don't word wrap.
			_fTempX += _fTempSpaceCharSize * _fTempFrameBufferXRescaleFactor;
			return TRUE;
		}
	} else {
		//THIS CHECKS BOTH RIGHT TOP AND BOTTOM TO MAKE SURE IT FALLS WITHIN THE X BOUNDARIES
		if( ( fLowerRightX < _fTempLowerRightInnerEdgeX ) &&
			( fUpperRightX < _fTempLowerRightInnerEdgeX ) ) {
			// Don't word wrap.
			_fTempX += ( ( _poTempLetter->fWidth + _poTempFont->fLetterSpacing ) * _fTempFontScale * _fTempFrameBufferXRescaleFactor );
			return TRUE;
		}
	}

	//IF WE ARE HERE THEN WE NEED TO WORD WRAP THIS WORD!
	
	//// Find deltas.
	//
	_fTempDeltaY = ( ( _poTempFont->fHeight * _fTempFontScale ) + ( _fTempResY *_poTempArea->oAttributes.fLineSpacing * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH ) );

	_fTempDeltaX = fLowerRightX;
	_poTempFont2 = _poFontsHead;

	//get the coordinates of the first letter of this word that needs to be wrapped.
	u32 nFirstLetterInWordIndex = _nLettersUsed - _nLettersInCurrentWord;
	
	f32 fFirstLetterUpperLeftX, fFirstLetterUpperRightX, fFirstLetterLowerLeftX, fFirstLetterLowerRightX;
	f32 fFirstLetterUpperY, fFirstLetterLowerY; //the y values will be the same between left and right
	ftext_CalculateLetterCoordinates( &FText_paoPrintfLetters[ nFirstLetterInWordIndex ], 
		&fFirstLetterUpperLeftX, &fFirstLetterUpperRightX,
		&fFirstLetterLowerLeftX, &fFirstLetterLowerRightX,
		&fFirstLetterUpperY, &fFirstLetterLowerY );

	//// Find the left-most X.
	//
	f32 fTemp1=0;
	if( _fTempDeltaX > fFirstLetterUpperLeftX ) {
		_fTempDeltaX = fFirstLetterUpperLeftX;
		fTemp1 = fFirstLetterUpperRightX - fFirstLetterUpperLeftX;
	}

	if( _fTempDeltaX > fFirstLetterLowerLeftX ) {
		_fTempDeltaX = fFirstLetterLowerLeftX;
		fTemp1 = fFirstLetterLowerRightX - fFirstLetterLowerLeftX;
	}
	//
	////

	if( _bTempFixedWidthFont ) {
		_fTempDeltaX -= ( ( ( _poTempFont->fWidth * _fTempFontScale * _fTempFrameBufferXRescaleFactor ) - fTemp1 ) * 0.5f );
	}

	_fTempDeltaX -= _fTempCursorOriginX;

	//// Maintain texel and pixel alignment.
	//
	if( ( _FTEXT_PRINTF_DEBUG_XY_ONLY == oType ) || _bTempOTOTexelPixelAlignment ) {
		_fTempDeltaX = (f32)( (s32)( _fTempDeltaX ) );
		_fTempDeltaY = (f32)( (s32)( _fTempDeltaY ) );
	}
	//
	////

	//Now apply the new deltas
	for( u32 uIdx = nFirstLetterInWordIndex; uIdx < _nLettersUsed; uIdx++ ) {
		FText_paoPrintfLetters[ uIdx ].fX -= _fTempDeltaX;
		FText_paoPrintfLetters[ uIdx ].fY += _fTempDeltaY;
	}

	//// Reposition cursor.
	//
	_fTempX -= _fTempDeltaX;
	_fTempY += _fTempDeltaY;

	_fTempX += ( _bTempFixedWidthFont ? ( _fTempSpaceCharSize * _fTempFrameBufferXRescaleFactor ) : ( ( _poTempLetter->fWidth + _poTempFont->fLetterSpacing ) * _fTempFontScale * _fTempFrameBufferXRescaleFactor ) );
	//
	////
	//
	//// Word wrapping.

	//now, we need re-align the previous line (the one we just wrapped past) if need be
	if( FTEXT_HORZ_ALIGN_LEFT == _oTempHorzAlign ) {
		_nLettersInCurrentLine = _nLettersInCurrentWord;
	} else {
		_CalculateHorizontalAlignmentOffset( oType, fX, TRUE );
	}

	// Culling. (Remove current word if wrapped beyond bottom of area.)
	if( ( fFirstLetterLowerY + _fTempDeltaY ) > _fTempLowerRightInnerEdgeY )
	{
		//we need to run through the characters that are being culled, and decrement
		//the instance counter in each texture page that the letter belongs to!
		
		FDataFntFile_Font_t *pTempFont = _poTempFont;
		for( u32 uIndx = nFirstLetterInWordIndex; uIndx < _nLettersUsed; uIndx++ ) {
			//find font if needed
			if( FText_paoPrintfLetters[ uIndx ].hFont != pTempFont->oHandle ) {
				FDataFntFile_Font_t *pTempFont2 = _poFontsHead;
				while( pTempFont2 ) {
					if( pTempFont2->oHandle == FText_paoPrintfLetters[ uIndx ].hFont ) {
						pTempFont = pTempFont2;
						break;
					}
					pTempFont2 = pTempFont2->poFontNext;
				}
			}
			// now decrease the texture page letter count
			FDataFntFile_Letter_t *pTempLetter = &pTempFont->paoFntLetters[ FText_paoPrintfLetters[ uIndx ].uFntLetterIdx ];
			pTempFont->pauTextLengthPerTexturePages[ pTempLetter->uTexurePage ]--;
		}
		_nLettersUsed = nFirstLetterInWordIndex;
		_nLettersInCurrentLine = 0;
		_nLettersInCurrentWord = 0;

		return FALSE;
	}

	//if we are here, we word wrapped successfully
	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void _CalculateHorizontalAlignmentOffset( const _FTextPrintf_e oType, const f32 fX, BOOL bWordWrapping ) {
	//// Find horizontal delta.
	//
	//// Find first _fTempDeltaX.
	//
	u32 uLettersToCenter = _nLettersInCurrentLine;
	if( bWordWrapping ) {
		uLettersToCenter -= _nLettersInCurrentWord;
	}

	f32 fTemp1 = 0.0f;
	if( uLettersToCenter ) {

		u32 nLastLetterIndex = _nLettersUsed - 1;
		if( bWordWrapping ) {
			// Last letter of previous word.
			nLastLetterIndex -= _nLettersInCurrentWord;
		}

		//// Find the right-most X.
		//
		f32 fLastLetterUpperLeftX, fLastLetterUpperRightX, fLastLetterLowerLeftX, fLastLetterLowerRightX;
		ftext_CalculateLetterCoordinates( &FText_paoPrintfLetters[ nLastLetterIndex ], &fLastLetterUpperLeftX, &fLastLetterUpperRightX,
			                         &fLastLetterLowerLeftX, &fLastLetterLowerRightX, NULL, NULL);

		_fTempDeltaX = fLastLetterLowerRightX;
		fTemp1 = fLastLetterLowerRightX - fLastLetterLowerLeftX;

		if( _fTempDeltaX < fLastLetterUpperRightX )
		{
			_fTempDeltaX = fLastLetterUpperRightX;
			fTemp1 = fLastLetterUpperRightX - fLastLetterUpperLeftX;
		}
	}

	if( _FTEXT_PRINTF_AREA_ONLY == oType )
	{
		_fTempDeltaX = _fTempLowerRightInnerEdgeX - _fTempDeltaX - 1.0f;
	}
	else if( _FTEXT_PRINTF_AREA_AND_XY == oType )
	{
		_fTempDeltaX = ( _fTempResX * ( _poTempArea->oAttributes.fUpperLeftX + _poTempArea->oAttributes.fBorderThicknessX + ( fX * ( _poTempArea->oAttributes.fLowerRightX - _poTempArea->oAttributes.fUpperLeftX - ( 2.0f * _poTempArea->oAttributes.fBorderThicknessX ) ) ) ) ) - _fTempDeltaX - 1.0f;
	}
	else // _FTEXT_PRINTF_XY_ONLY || _FTEXT_PRINTF_DEBUG_XY_ONLY
	{
		_fTempDeltaX = ( _fTempResX * fX ) - _fTempDeltaX - 1.0f;
	}

	if( _bTempFixedWidthFont )
	{
		_fTempDeltaX -= ( ( ( _poTempFont->fWidth * _fTempFontScale * _fTempFrameBufferXRescaleFactor ) - fTemp1 ) * 0.5f );
	}

	if( FTEXT_HORZ_ALIGN_CENTER == _oTempHorzAlign )
	{
		_fTempDeltaX *= ( 0.5f );
	}
	//
	//// Find horizontal delta.

	//// Maintain texel and pixel alignment.
	//
	if( ( _FTEXT_PRINTF_DEBUG_XY_ONLY == oType ) || _bTempOTOTexelPixelAlignment )
	{
		_fTempDeltaX = (f32)( (s32)( _fTempDeltaX ) );
		_fTempDeltaY = (f32)( (s32)( _fTempDeltaY ) );
	}
	//
	////


	u32 nStartIdx = _nLettersUsed - _nLettersInCurrentLine;

	//Now apply the new deltas
	for( u32 uIdx = 0; uIdx < uLettersToCenter; uIdx++ ) {
		FText_paoPrintfLetters[ nStartIdx + uIdx ].fX += _fTempDeltaX;
	}

	if( bWordWrapping ) {
		//SETS THE NEW LINE LENGTH TO THE WORD THAT WAS WRAPPED
		_nLettersInCurrentLine = _nLettersInCurrentWord;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL ftext_IsFontLoaded( cchar *pszName )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );
	FASSERT_MSG( pszName,           "[ FTEXT ] Error: NULL pointer !!!" );

	_poTempFont = _poFontsHead->poFontNext;

	while( _poTempFont )
	{
		if( 0 == fclib_strcmp( (char *)_poTempFont->szName, pszName ) )
		{
			return TRUE;
		}

		_poTempFont = _poTempFont->poFontNext;
	}

	return FALSE;

} // ftext_IsFontLoaded

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FTextError_e ftext_Load( FDataFntFile_Handle_t ohFont, cchar *pszName )
{
	////
	//
	FASSERT_MSG( _bModuleInstalled,     "[ FTEXT ] Error: System not installed !!!" );
	FASSERT_MSG( ohFont,                "[ FTEXT ] Error: NULL handle !!!" );
	FASSERT_MSG( pszName,               "[ FTEXT ] Error: NULL pointer !!!" );

#if( ! FANG_PRODUCTION_BUILD )
	_poTempFont = _poFontsHead;

	do
	{
		if( 0 == fclib_strcmp( (char *)_poTempFont->szName, pszName ) )
		{
			if( _poTempFont->oHandle == ohFont )
			{
				DEVPRINTF( "[ FTEXT ] Warning %u: Font \"%s\" already loaded !!!\n", __LINE__, pszName );
				return FTEXT_NO_ERROR;
			}
			else
			{
				DEVPRINTF( "[ FTEXT ] Error %u: Can't load font \"%s\" (already loaded) !!!\n", __LINE__, pszName );
				return FTEXT_ERROR;
			}
		}

		_poTempFont = _poTempFont->poFontNext;

	} while( _poTempFont );
#endif
	//
	////

	//// Open font definition file.
	//
	sprintf( (char *)_pszTemp, "%s.fnt", pszName );
	FFileHandle ohFile = ffile_Open( (char *)_pszTemp, FFILE_OPEN_RONLY );
	if( FFILE_INVALID_HANDLE == ohFile )
	{
		DEVPRINTF( "[ FTEXT ] Error %u: Could not open font (\"%s\") definition file !!!\n", __LINE__, _pszTemp );
		return FTEXT_ERROR;
	}
	//
	////

	FResFrame_t ohTempResFrame = fres_GetFrame();

	u32 uTotalBytesAllocatedNontexture, uTotalBytesAllocatedTexture;
	FResFrame_t ohTempDiffFrame1, ohTempDiffFrame2;

	//// Read and alloc mem for font info.
	//
	uTotalBytesAllocatedNontexture = ffile_GetFileSize( ohFile );
	if( ! uTotalBytesAllocatedNontexture )
	{
		DEVPRINTF( "[ FTEXT ] Error %u: Invalid font file !!!\n", __LINE__ );
		ffile_Close( ohFile );
		return FTEXT_ERROR;
	}

	// Create an fres resource...
	FResHandle_t hRes = fres_CreateWithCallback( NULL, NULL, _ResFontDestroy );
	if( hRes == FRES_NULLHANDLE ) {
		DEVPRINTF( "ftext_Load(): Could not create ftext resource object.\n" );
		ffile_Close( ohFile );
		return FTEXT_ERROR;
	}

	FDataFntFile_Font_t *poNewFont = (FDataFntFile_Font_t *)fres_Alloc( uTotalBytesAllocatedNontexture );
	if( ! poNewFont )
	{
		DEVPRINTF( "[ FTEXT ] Error %u: Can't allocate memory for font !!!\n", __LINE__ );
		ffile_Close( ohFile );
		return FTEXT_ERROR;
	}

	// Set resource base pointer...
	fres_SetBase( hRes, poNewFont );

	if( (s32)uTotalBytesAllocatedNontexture != ffile_Read( ohFile, uTotalBytesAllocatedNontexture, poNewFont ) )
	{
		DEVPRINTF( "[ FTEXT ] Error %u: Can't read font !!!\n", __LINE__ );
		ffile_Close( ohFile );
		fres_ReleaseFrame( ohTempResFrame );
		return FTEXT_ERROR;
	}

	ffile_Close( ohFile );
	//
	////

	poNewFont->oHandle = ohFont;

	//// Fixup ( offsets -> pointers ).
	//
	poNewFont->paoLetterBuckets = (FDataFntFile_LetterBucket_t *)( (u32)poNewFont->paoLetterBuckets + (u32)poNewFont );
	poNewFont->paoBucketLetters = (FDataFntFile_BucketLetter_t *)( (u32)poNewFont->paoBucketLetters + (u32)poNewFont );
	poNewFont->paoFntLetters = (FDataFntFile_Letter_t *)( (u32)poNewFont->paoFntLetters + (u32)poNewFont );
	//
	////

	//// Alloc mem for texture resource pointers.
	//
	poNewFont->paoTexInsts = (CFTexInst *)fres_AllocAndZero( poNewFont->uTexPages * sizeof( CFTexInst ) );
	if( ! poNewFont->paoTexInsts )
	{
		fres_ReleaseFrame( ohTempResFrame );
		DEVPRINTF( "[ FTEXT ] Error %u: Could not allocate memory necessary for font.\n", __LINE__ );
		return FTEXT_ERROR;
	}
	uTotalBytesAllocatedNontexture += ( poNewFont->uTexPages * sizeof( FTexDef_t * ) );
	//
	////

	//// Load font textures.
	//
	uTotalBytesAllocatedTexture = 0;
	ohTempDiffFrame1 = fres_GetFrame();

	for( _uIndex = 0; _uIndex < poNewFont->uTexPages; ++_uIndex )
	{
		sprintf( (char *)_pszTemp, "%s%u", poNewFont->szName, _uIndex );

		poNewFont->paoTexInsts[ _uIndex ].SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, (char *)_pszTemp ) );

		if( ! poNewFont->paoTexInsts[ _uIndex ].GetTexDef() )
		{
			fres_ReleaseFrame( ohTempResFrame );
			DEVPRINTF( "[ FTEXT ] Error %u : Could not load font texture \"%s\" !!!\n", __LINE__, _pszTemp );
			return FTEXT_ERROR;
		}

		//test to disable filtering for minification
		//poNewFont->paoTexInsts[ _uIndex ].SetFlags( CFTexInst::FLAG_MAGFILTER_NO_BILINEAR | CFTexInst::FLAG_MINFILTER_NO_TRILINEAR | CFTexInst::FLAG_MINFILTER_NO_BILINEAR );

	}

	ohTempDiffFrame2 = fres_GetFrame();
	uTotalBytesAllocatedTexture += ( (u32)ohTempDiffFrame1 - (u32)ohTempDiffFrame2 );
	//
	////

	//// Alloc mem for font data.
	//
	poNewFont->pauTextLengthPerTexturePages = (u32 *)fres_AllocAndZero( poNewFont->uTexPages * sizeof( u32 ) );

	if( !poNewFont->pauTextLengthPerTexturePages  )
	{
		fres_ReleaseFrame( ohTempResFrame );
		DEVPRINTF( "[ FTEXT ] Error %u: Could not allocate memory necessary to load font.\n", __LINE__ );
		return FTEXT_ERROR;
	}

	uTotalBytesAllocatedNontexture += ( poNewFont->uTexPages * sizeof( u32 ) ); // pauTextLengthPerTexturePages

	for( _uIndex = 0; _uIndex < poNewFont->uTexPages; ++_uIndex )
	{
		poNewFont->pauTextLengthPerTexturePages[ _uIndex ] = 0;
	}
	//
	////

	//// Add new font to end of list.
	//
	_poTempFont = _poFontsHead;

	while( _poTempFont->poFontNext )
	{
		_poTempFont = _poTempFont->poFontNext;
	}

	_poTempFont->poFontNext = poNewFont;
	//
	////

//	DEVPRINTF( "[ FTEXT ] Memory usage for \"%-11s\" (in bytes): %4u (tex) + %6u (!tex) = %6u\n", pszName, uTotalBytesAllocatedTexture, uTotalBytesAllocatedNontexture, ( uTotalBytesAllocatedTexture + uTotalBytesAllocatedNontexture ) );
	DEVPRINTF( "[ FTEXT ] Memory usage for \"%-11s\" (in bytes): %6u\n", pszName, uTotalBytesAllocatedNontexture );

	return FTEXT_NO_ERROR;

} // ftext_Load

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_ClearAllPending( void )
{
	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

	//// Clear each font.
	//
	_poTempFont = _poFontsHead;

	do
	{
		for( _uIndex = 0; _uIndex < _poTempFont->uTexPages; ++_uIndex )
		{
			if( ! _poTempFont->pauTextLengthPerTexturePages[ _uIndex ] )
			{
				continue;
			}

			// Reset texture page.
			_poTempFont->pauTextLengthPerTexturePages[ _uIndex ] = 0;
		}

		_poTempFont = _poTempFont->poFontNext;

	} while( _poTempFont );
	//
	////

	//// Clean up.
	//
	if( _poAreasHead )
	{
		_poTempArea = _poAreasHead;

		do
		{
			_poTempArea->bAreaDrawn = FALSE;
			_poTempArea->bBorderDrawn = FALSE;
			_poTempArea->fUnitCursorY = 0.0f;

			_poTempArea = _poTempArea->poAreaNext;

		} while( _poTempArea );
	}
	//
	////

	_nLettersUsed = 0;
} // ftext_ClearAllPending

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ftext_Draw( void )
{
//	FASSERT_MSG( _bModuleInstalled, "[ FTEXT ] Error: System not installed !!!" );

	if( ! _bModuleInstalled )
	{
		return;
	}

#if( _FTEXT_STUB_ONLY_RUNTIME_OUT )
	return;
#endif

#if _VERTS_USAGE_STATS
	//// Usage statistics.
	//
	_uStatVertsUsedCurrentDraw = 0;
	//
	////
#endif

	_poViewportPrevious = fviewport_GetActive();

	//// Setup viewport.
	//
	frenderer_Push( FRENDERER_DRAW, NULL );
	//
	////

	//// Setup camera.
	//
	_oCamera.InitStackWithView();
	//
	////

	//// Setup draw options.
	//
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	//
	////

	fviewport_SetActive( _poViewportDefault );

	//// Draw each area.
	//
	_poTempArea = _poAreasHead;

	//Allocate a pool for the Area/Border and Guides
	FDrawVtx_t *paoAreaAndBorderVertsBuffer;
	paoAreaAndBorderVertsBuffer = fvtxpool_GetArray( 10, TRUE );
	if( !paoAreaAndBorderVertsBuffer ) {
		DEVPRINTF( "[ FTEXT ] Could not allocate any vertices from the fvtxpool to render the area borders -- aborting rendering!\n" );
		goto _FText_Draw_Exit;
	}
	FDrawVtx_t *paoDrawGuidesVertsBuffer;
	paoDrawGuidesVertsBuffer = fvtxpool_GetArray( 2, TRUE );
	if( !paoDrawGuidesVertsBuffer ) {
		DEVPRINTF( "[ FTEXT ] Could not allocate any vertices from the fvtxpool to render area guides -- aborting rendering!\n" );
		goto _FText_Draw_Exit;
	}

#if 1
	do
	{
		_fTempGeneralAlpha = _poTempArea->oAttributes.oColorBackground.fAlpha * _poTempArea->oAttributes.fAlpha;
		if( ( _poTempArea->oAttributes.bVisible ) &&
			( ! _poTempArea->bAreaDrawn ) &&
			( 0.01f <= _fTempGeneralAlpha ) )
		{
			//// Setup texture.
			//
			if( _poTempArea->oTexInst.GetTexDef() )
			{
				fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
			}
			else
			{
				fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
			}

			fdraw_SetTexture( &_poTempArea->oTexInst );
			//
			////

			//// Verts.
			//
			paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA = _poTempArea->oAttributes.oColorBackground;
			paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA.fAlpha = _fTempGeneralAlpha;
			paoAreaAndBorderVertsBuffer[ 1 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
			paoAreaAndBorderVertsBuffer[ 2 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
			paoAreaAndBorderVertsBuffer[ 3 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;

			f32 fLeft = _poTempArea->oVpRect.UpperLeft.x + (_poTempArea->oVpRes.x * (_poTempArea->oAttributes.fUpperLeftX  + _poTempArea->oAttributes.fBorderThicknessX));
			f32 fTop  = _poTempArea->oVpRect.UpperLeft.y + (_poTempArea->oVpRes.y * (_poTempArea->oAttributes.fUpperLeftY  + _poTempArea->oAttributes.fBorderThicknessY) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH);
			f32 fRight = _poTempArea->oVpRect.UpperLeft.x + (_poTempArea->oVpRes.x * (_poTempArea->oAttributes.fLowerRightX - _poTempArea->oAttributes.fBorderThicknessX));
			f32 fBottom = _poTempArea->oVpRect.UpperLeft.y + (_poTempArea->oVpRes.y * (_poTempArea->oAttributes.fLowerRightY - _poTempArea->oAttributes.fBorderThicknessY) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH);

			paoAreaAndBorderVertsBuffer[ 0 ].Pos_MS.Set( fLeft, fTop, 1.0f );
			paoAreaAndBorderVertsBuffer[ 3 ].Pos_MS.Set( fRight, fBottom, 1.0f );
			paoAreaAndBorderVertsBuffer[ 1 ].Pos_MS.Set( paoAreaAndBorderVertsBuffer[ 3 ].Pos_MS.x, paoAreaAndBorderVertsBuffer[ 0 ].Pos_MS.y, 1.0f );
			paoAreaAndBorderVertsBuffer[ 2 ].Pos_MS.Set( paoAreaAndBorderVertsBuffer[ 0 ].Pos_MS.x, paoAreaAndBorderVertsBuffer[ 3 ].Pos_MS.y, 1.0f );

			paoAreaAndBorderVertsBuffer[ 0 ].ST.Set( _poTempArea->oAttributes.fUVUpperLeftX,  _poTempArea->oAttributes.fUVUpperLeftY  );
			paoAreaAndBorderVertsBuffer[ 1 ].ST.Set( _poTempArea->oAttributes.fUVLowerRightX, _poTempArea->oAttributes.fUVUpperLeftY  );
			paoAreaAndBorderVertsBuffer[ 2 ].ST.Set( _poTempArea->oAttributes.fUVUpperLeftX,  _poTempArea->oAttributes.fUVLowerRightY );
			paoAreaAndBorderVertsBuffer[ 3 ].ST.Set( _poTempArea->oAttributes.fUVLowerRightX, _poTempArea->oAttributes.fUVLowerRightY );
			//
			////

			//// Draw.
			//
			fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, paoAreaAndBorderVertsBuffer, 4 );

#if _VERTS_USAGE_STATS
			//// Usage statistics.
			//
			_uStatVertsUsedCurrentDraw += 4;
			//
			////
#endif

			_poTempArea->bAreaDrawn = TRUE;
			//
			//// Draw.

			// Draw all other areas using the same texture.
			_poTempArea2 = _poTempArea->poAreaNext;
			while( _poTempArea2 )
			{
				_fTempGeneralAlpha = _poTempArea2->oAttributes.oColorBackground.fAlpha * _poTempArea2->oAttributes.fAlpha;
				if( ( _poTempArea2->oAttributes.bVisible ) &&
					( ! _poTempArea2->bAreaDrawn ) &&
					( _poTempArea->oTexInst.GetTexDef() == _poTempArea2->oTexInst.GetTexDef() ) &&
					( 0.01f <= _fTempGeneralAlpha ) )
				{

					//// Verts.
					//
					paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA = _poTempArea2->oAttributes.oColorBackground;
					paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA.fAlpha = _fTempGeneralAlpha;
					paoAreaAndBorderVertsBuffer[ 1 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
					paoAreaAndBorderVertsBuffer[ 2 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
					paoAreaAndBorderVertsBuffer[ 3 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;

					f32 fLeft   = _poTempArea2->oVpRect.UpperLeft.x + (_poTempArea2->oVpRes.x * (_poTempArea2->oAttributes.fUpperLeftX  + _poTempArea2->oAttributes.fBorderThicknessX));
					f32 fTop    = _poTempArea2->oVpRect.UpperLeft.y + (_poTempArea2->oVpRes.y * (_poTempArea2->oAttributes.fUpperLeftY  + _poTempArea2->oAttributes.fBorderThicknessY) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH);
					f32 fRight  = _poTempArea2->oVpRect.UpperLeft.x + (_poTempArea2->oVpRes.x * (_poTempArea2->oAttributes.fLowerRightX - _poTempArea2->oAttributes.fBorderThicknessX));
					f32 fBottom = _poTempArea2->oVpRect.UpperLeft.y + (_poTempArea2->oVpRes.y * (_poTempArea2->oAttributes.fLowerRightY - _poTempArea2->oAttributes.fBorderThicknessY) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH);

					paoAreaAndBorderVertsBuffer[ 0 ].Pos_MS.Set( fLeft, fTop, 1.0f );
					paoAreaAndBorderVertsBuffer[ 3 ].Pos_MS.Set( fRight, fBottom, 1.0f );
					paoAreaAndBorderVertsBuffer[ 1 ].Pos_MS.Set( paoAreaAndBorderVertsBuffer[ 3 ].Pos_MS.x, paoAreaAndBorderVertsBuffer[ 0 ].Pos_MS.y, 1.0f );
					paoAreaAndBorderVertsBuffer[ 2 ].Pos_MS.Set( paoAreaAndBorderVertsBuffer[ 0 ].Pos_MS.x, paoAreaAndBorderVertsBuffer[ 3 ].Pos_MS.y, 1.0f );

					paoAreaAndBorderVertsBuffer[ 0 ].ST.Set( _poTempArea2->oAttributes.fUVUpperLeftX,  _poTempArea2->oAttributes.fUVUpperLeftY  );
					paoAreaAndBorderVertsBuffer[ 1 ].ST.Set( _poTempArea2->oAttributes.fUVLowerRightX, _poTempArea2->oAttributes.fUVUpperLeftY  );
					paoAreaAndBorderVertsBuffer[ 2 ].ST.Set( _poTempArea2->oAttributes.fUVUpperLeftX,  _poTempArea2->oAttributes.fUVLowerRightY );
					paoAreaAndBorderVertsBuffer[ 3 ].ST.Set( _poTempArea2->oAttributes.fUVLowerRightX, _poTempArea2->oAttributes.fUVLowerRightY );
					//
					////

					//// Draw.
					//
					fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, paoAreaAndBorderVertsBuffer, 4 );
					_poTempArea2->bAreaDrawn = TRUE;

#if _VERTS_USAGE_STATS
					//// Usage statistics.
					//
					_uStatVertsUsedCurrentDraw += 4;
					//
					////
#endif

					//
					//// Draw.
				}

				_poTempArea2 = _poTempArea2->poAreaNext;
			}
		}

		_poTempArea = _poTempArea->poAreaNext;

	} while( _poTempArea );
	//
	//// Draw each area.
#endif

	//// Draw borders for each area.
	//
	fdraw_SetTexture( NULL );

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );

	_poTempArea = _poAreasHead;

#if 1
	do
	{
		_fTempGeneralAlpha = _poTempArea->oAttributes.oColorBorder.fAlpha * _poTempArea->oAttributes.fAlpha;
		if( ( _poTempArea->oAttributes.bVisible ) &&
			( 0.01f <= _fTempGeneralAlpha ) &&
			( ! _poTempArea->bBorderDrawn ) )
/*
		if( ( _poTempArea->oAttributes.bVisible ) &&
			( 0.01f <= _fTempGeneralAlpha ) &&
			( ! _poTempArea->bBorderDrawn ) &&
			( 0.001f <= _poTempArea->oAttributes.fBorderThicknessX ) &&
			( 0.001f <= _poTempArea->oAttributes.fBorderThicknessY ) )
*/
		{
			//// Verts.
			//
			paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA = _poTempArea->oAttributes.oColorBorder;
			paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA.fAlpha = _fTempGeneralAlpha;
			paoAreaAndBorderVertsBuffer[ 1 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
			paoAreaAndBorderVertsBuffer[ 2 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
			paoAreaAndBorderVertsBuffer[ 3 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
			paoAreaAndBorderVertsBuffer[ 4 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
			paoAreaAndBorderVertsBuffer[ 5 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
			paoAreaAndBorderVertsBuffer[ 6 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
			paoAreaAndBorderVertsBuffer[ 7 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
			paoAreaAndBorderVertsBuffer[ 8 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
			paoAreaAndBorderVertsBuffer[ 9 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;

			CFRect2D& rRect = _poTempArea->oVpRect;
			CFVec2&   rRes  = _poTempArea->oVpRes;

			paoAreaAndBorderVertsBuffer[ 8 ].Pos_MS = paoAreaAndBorderVertsBuffer[ 0 ].Pos_MS.Set( rRect.UpperLeft.x + (rRes.x * _poTempArea->oAttributes.fUpperLeftX), rRect.UpperLeft.y + (rRes.y * _poTempArea->oAttributes.fUpperLeftY) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH, 1.0f );
			paoAreaAndBorderVertsBuffer[ 9 ].Pos_MS = paoAreaAndBorderVertsBuffer[ 1 ].Pos_MS.Set( rRect.UpperLeft.x + (rRes.x * (_poTempArea->oAttributes.fUpperLeftX + _poTempArea->oAttributes.fBorderThicknessX)), rRect.m_UpperLeft.y + (rRes.y * (_poTempArea->oAttributes.fUpperLeftY + _poTempArea->oAttributes.fBorderThicknessY ) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH), 1.0f );
			paoAreaAndBorderVertsBuffer[ 4 ].Pos_MS.Set( rRect.UpperLeft.x + (rRes.x * _poTempArea->oAttributes.fLowerRightX), rRect.UpperLeft.y + (rRes.y * _poTempArea->oAttributes.fLowerRightY) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH, 1.0f );
			paoAreaAndBorderVertsBuffer[ 5 ].Pos_MS.Set( rRect.UpperLeft.x + (rRes.x * ( _poTempArea->oAttributes.fLowerRightX - _poTempArea->oAttributes.fBorderThicknessX )), rRect.UpperLeft.y + rRes.y * ( _poTempArea->oAttributes.fLowerRightY - _poTempArea->oAttributes.fBorderThicknessY ) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH, 1.0f );
			paoAreaAndBorderVertsBuffer[ 2 ].Pos_MS.Set( paoAreaAndBorderVertsBuffer[ 4 ].Pos_MS.x, paoAreaAndBorderVertsBuffer[ 0 ].Pos_MS.y, 1.0f );
			paoAreaAndBorderVertsBuffer[ 3 ].Pos_MS.Set( paoAreaAndBorderVertsBuffer[ 5 ].Pos_MS.x, paoAreaAndBorderVertsBuffer[ 1 ].Pos_MS.y, 1.0f );
			paoAreaAndBorderVertsBuffer[ 6 ].Pos_MS.Set( paoAreaAndBorderVertsBuffer[ 0 ].Pos_MS.x, paoAreaAndBorderVertsBuffer[ 4 ].Pos_MS.y, 1.0f );
			paoAreaAndBorderVertsBuffer[ 7 ].Pos_MS.Set( paoAreaAndBorderVertsBuffer[ 1 ].Pos_MS.x, paoAreaAndBorderVertsBuffer[ 5 ].Pos_MS.y, 1.0f );
			//
			////

			//// Draw.
			//
			fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, paoAreaAndBorderVertsBuffer, 10 );
			_poTempArea->bBorderDrawn = TRUE;

#if _VERTS_USAGE_STATS
			//// Usage statistics.
			//
			_uStatVertsUsedCurrentDraw += 10;
			//
			////
#endif

			//
			//// Draw.

			//// Guides.
			//
			if( _poTempArea->oAttributes.bDrawGuides )
			{
				//// Set current font.
				//
				_poTempFont = _poFontsHead;

				do
				{
					if( _poTempFont->oHandle == _poTempArea->oAttributes.ohFont )
					{
						break;
					}

					_poTempFont = _poTempFont->poFontNext;

				} while( _poTempFont );
				//
				////

				//// Verts.
				//
				paoDrawGuidesVertsBuffer[ 0 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
				paoDrawGuidesVertsBuffer[ 1 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;

				paoDrawGuidesVertsBuffer[ 0 ].Pos_MS.Set( paoAreaAndBorderVertsBuffer[ 1 ].Pos_MS.x, 0.0f, 1.0f ); // 0.0f -> placeholder.
				paoDrawGuidesVertsBuffer[ 1 ].Pos_MS.Set( paoAreaAndBorderVertsBuffer[ 3 ].Pos_MS.x, 0.0f, 1.0f ); // 0.0f -> placeholder.
				//
				////

				_fTempY = ( _fTempResY * ( _poTempArea->oAttributes.fUpperLeftY + _poTempArea->oAttributes.fBorderThicknessY + ( _poTempArea->oAttributes.fLineSpacing * 0.5f ) ) ) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH;

				_fTempFontScale = fmath_Div( ( fmath_Div( ( _poTempArea->oAttributes.fLowerRightY - ( _poTempArea->oAttributes.fUpperLeftY + ( 2 * _poTempArea->oAttributes.fBorderThicknessY ) + ( _poTempArea->oAttributes.fNumberOfLines * _poTempArea->oAttributes.fLineSpacing ) ) ), _poTempArea->oAttributes.fNumberOfLines ) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH * _fTempResY ), _poTempFont->fHeight );

				do
				{
					paoDrawGuidesVertsBuffer[ 0 ].Pos_MS.y = _fTempY + ( _fTempFontScale * _poTempFont->fVerticalRuler );
					paoDrawGuidesVertsBuffer[ 1 ].Pos_MS.y = paoDrawGuidesVertsBuffer[ 0 ].Pos_MS.y;

					fdraw_Line( &( paoDrawGuidesVertsBuffer[ 0 ] ), &( paoDrawGuidesVertsBuffer[ 1 ] ) );

#if _VERTS_USAGE_STATS
					//// Usage statistics.
					//
					_uStatVertsUsedCurrentDraw += 2;
					//
					////
#endif

					_fTempY += ( ( _fTempFontScale * _poTempFont->fHeight ) + ( _fTempResY * _poTempArea->oAttributes.fLineSpacing ) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH );

				} while( _fTempY < paoAreaAndBorderVertsBuffer[ 5 ].Pos_MS.y );
			}
			//
			////

			// Draw all other borders using the same viewport.
			_poTempArea2 = _poTempArea->poAreaNext;
			while( _poTempArea2 )
			{
				_fTempGeneralAlpha = _poTempArea2->oAttributes.oColorBorder.fAlpha * _poTempArea2->oAttributes.fAlpha;
				if( ( _poTempArea2->oAttributes.bVisible ) &&
					( 0.01f <= _fTempGeneralAlpha ) &&
					( ! _poTempArea2->bBorderDrawn ) )
/*
				if( ( _poTempArea2->oAttributes.bVisible ) &&
					( 0.01f <= _fTempGeneralAlpha ) &&
					( ! _poTempArea2->bBorderDrawn ) &&
					( _poTempArea->oAttributes.poViewport == _poTempArea2->oAttributes.poViewport ) &&
					( 0.001f <= _poTempArea2->oAttributes.fBorderThicknessX ) &&
					( 0.001f <= _poTempArea2->oAttributes.fBorderThicknessY ) )
*/
				{
					//// Verts.
					//
					paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA = _poTempArea2->oAttributes.oColorBorder;
					paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA.fAlpha = _fTempGeneralAlpha;
					paoAreaAndBorderVertsBuffer[ 1 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
					paoAreaAndBorderVertsBuffer[ 2 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
					paoAreaAndBorderVertsBuffer[ 3 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
					paoAreaAndBorderVertsBuffer[ 4 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
					paoAreaAndBorderVertsBuffer[ 5 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
					paoAreaAndBorderVertsBuffer[ 6 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
					paoAreaAndBorderVertsBuffer[ 7 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
					paoAreaAndBorderVertsBuffer[ 8 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
					paoAreaAndBorderVertsBuffer[ 9 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;

					paoAreaAndBorderVertsBuffer[ 8 ].Pos_MS = paoAreaAndBorderVertsBuffer[ 0 ].Pos_MS.Set( _poTempArea2->oVpRect.UpperLeft.x + _poTempArea2->oVpRes.x * _poTempArea2->oAttributes.fUpperLeftX, _poTempArea2->oVpRect.UpperLeft.y + _poTempArea2->oVpRes.y * _poTempArea2->oAttributes.fUpperLeftY * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH, 1.0f );
					paoAreaAndBorderVertsBuffer[ 9 ].Pos_MS = paoAreaAndBorderVertsBuffer[ 1 ].Pos_MS.Set( _poTempArea2->oVpRect.UpperLeft.x + _poTempArea2->oVpRes.x * ( _poTempArea2->oAttributes.fUpperLeftX + _poTempArea2->oAttributes.fBorderThicknessX ), _poTempArea2->oVpRect.UpperLeft.y + _poTempArea2->oVpRes.y * ( _poTempArea2->oAttributes.fUpperLeftY  + _poTempArea2->oAttributes.fBorderThicknessY ) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH, 1.0f );
					paoAreaAndBorderVertsBuffer[ 4 ].Pos_MS.Set( _poTempArea2->oVpRect.UpperLeft.x + _poTempArea2->oVpRes.x * _poTempArea2->oAttributes.fLowerRightX, _poTempArea2->oVpRect.UpperLeft.y + _poTempArea2->oVpRes.y * _poTempArea2->oAttributes.fLowerRightY * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH, 1.0f );
					paoAreaAndBorderVertsBuffer[ 5 ].Pos_MS.Set( _poTempArea2->oVpRect.UpperLeft.x + _poTempArea2->oVpRes.x * ( _poTempArea2->oAttributes.fLowerRightX - _poTempArea2->oAttributes.fBorderThicknessX ), _poTempArea2->oVpRect.UpperLeft.y + _poTempArea2->oVpRes.y * ( _poTempArea2->oAttributes.fLowerRightY - _poTempArea2->oAttributes.fBorderThicknessY ) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH, 1.0f );

					paoAreaAndBorderVertsBuffer[ 2 ].Pos_MS.Set( paoAreaAndBorderVertsBuffer[ 4 ].Pos_MS.x, paoAreaAndBorderVertsBuffer[ 0 ].Pos_MS.y, 1.0f );
					paoAreaAndBorderVertsBuffer[ 3 ].Pos_MS.Set( paoAreaAndBorderVertsBuffer[ 5 ].Pos_MS.x, paoAreaAndBorderVertsBuffer[ 1 ].Pos_MS.y, 1.0f );
					paoAreaAndBorderVertsBuffer[ 6 ].Pos_MS.Set( paoAreaAndBorderVertsBuffer[ 0 ].Pos_MS.x, paoAreaAndBorderVertsBuffer[ 4 ].Pos_MS.y, 1.0f );
					paoAreaAndBorderVertsBuffer[ 7 ].Pos_MS.Set( paoAreaAndBorderVertsBuffer[ 1 ].Pos_MS.x, paoAreaAndBorderVertsBuffer[ 5 ].Pos_MS.y, 1.0f );
					//
					////

					//// Draw.
					//
					fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, paoAreaAndBorderVertsBuffer, 10 );
					_poTempArea2->bBorderDrawn = TRUE;

#if _VERTS_USAGE_STATS
					//// Usage statistics.
					//
					_uStatVertsUsedCurrentDraw += 10;
					//
					////
#endif

					//
					//// Draw.

					//// Guides.
					//
					if( _poTempArea->oAttributes.bDrawGuides )
					{
						//// Set current font.
						//
						_poTempFont = _poFontsHead;

						do
						{
							if( _poTempFont->oHandle == _poTempArea->oAttributes.ohFont )
							{
								break;
							}

							_poTempFont = _poTempFont->poFontNext;

						} while( _poTempFont );
						//
						////

						//// Verts.
						//
						paoDrawGuidesVertsBuffer[ 0 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;
						paoDrawGuidesVertsBuffer[ 1 ].ColorRGBA = paoAreaAndBorderVertsBuffer[ 0 ].ColorRGBA;

						paoDrawGuidesVertsBuffer[ 0 ].Pos_MS.Set( paoAreaAndBorderVertsBuffer[ 1 ].Pos_MS.x, 0.0f, 1.0f ); // 0.0f -> placeholder.
						paoDrawGuidesVertsBuffer[ 1 ].Pos_MS.Set( paoAreaAndBorderVertsBuffer[ 3 ].Pos_MS.x, 0.0f, 1.0f ); // 0.0f -> placeholder.
						//
						////

						_fTempY = ( _fTempResY * ( _poTempArea->oAttributes.fUpperLeftY + _poTempArea->oAttributes.fBorderThicknessY + ( _poTempArea->oAttributes.fLineSpacing * 0.5f ) ) ) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH;

						_fTempFontScale = fmath_Div( ( fmath_Div( ( _poTempArea->oAttributes.fLowerRightY - ( _poTempArea->oAttributes.fUpperLeftY + ( 2 * _poTempArea->oAttributes.fBorderThicknessY ) + ( _poTempArea->oAttributes.fNumberOfLines * _poTempArea->oAttributes.fLineSpacing ) ) ), _poTempArea->oAttributes.fNumberOfLines ) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH * _fTempResY ), _poTempFont->fHeight );

						do
						{
							paoDrawGuidesVertsBuffer[ 0 ].Pos_MS.y = _fTempY + ( _fTempFontScale * _poTempFont->fVerticalRuler );
							paoDrawGuidesVertsBuffer[ 1 ].Pos_MS.y = paoDrawGuidesVertsBuffer[ 0 ].Pos_MS.y;

							fdraw_Line( &( paoDrawGuidesVertsBuffer[ 0 ] ), &( paoDrawGuidesVertsBuffer[ 1 ] ) );

#if _VERTS_USAGE_STATS
							//// Usage statistics.
							//
							_uStatVertsUsedCurrentDraw += 2;
							//
							////
#endif

							_fTempY += ( ( _fTempFontScale * _poTempFont->fHeight ) + ( _fTempResY * _poTempArea->oAttributes.fLineSpacing ) * _PHYSICAL_DEVICE_ASPECT_RATIO_WoH );

						} while( _fTempY < paoAreaAndBorderVertsBuffer[ 5 ].Pos_MS.y );
					}
					//
					////
				}

				_poTempArea2 = _poTempArea2->poAreaNext;
			}
		}

		_poTempArea = _poTempArea->poAreaNext;

	} while( _poTempArea );
	//
	//// Draw borders for each area.
#endif


	fvtxpool_ReturnArray( paoAreaAndBorderVertsBuffer );
	fvtxpool_ReturnArray( paoDrawGuidesVertsBuffer );

	//// Draw each font.
	//
	_poTempFont = _poFontsHead;
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
    
	do
	{
		//run through each texture page for each font, and see if
		//there are any characters from this texture page that need to be rendered
		for( _uIndex = 0; _uIndex < _poTempFont->uTexPages; ++_uIndex )
		{
			if( _poTempFont->pauTextLengthPerTexturePages[ _uIndex ] )
			{
				ftext_DrawTexturePageCharacters( _poTempFont, _uIndex );
			}

			// Reset texture character counter.
			_poTempFont->pauTextLengthPerTexturePages[ _uIndex ] = 0;
		}

		_poTempFont = _poTempFont->poFontNext;

	} while( _poTempFont );
	//
	//// Draw each font.

#if _FTEXT_SHOW_TEXT_SAFE_AREA_OUTLINE_640_480
	////
	//
	FDraw_Vtx *paoTextSafeAreaVertsBuffer = fvtxpool_GetArray( 4, TRUE );
	if( !paoTextSafeAreaVertsBuffer ) {
		DEVPRINTF( "[ FTEXT ] Could not allocate any vertices from the fvtxpool to render the safe area borders -- aborting rendering!\n" );
		goto _FText_Draw_Exit;
	}

	fdraw_SetTexture( NULL );

	paoTextSafeAreaVertsBuffer[ 0 ].ColorRGBA.OpaqueWhite();
	paoTextSafeAreaVertsBuffer[ 1 ].ColorRGBA.OpaqueWhite();
	paoTextSafeAreaVertsBuffer[ 2 ].ColorRGBA.OpaqueWhite();
	paoTextSafeAreaVertsBuffer[ 3 ].ColorRGBA.OpaqueWhite();
/*
	paoTextSafeAreaVertsBuffer[ 0 ].Pos_MS.Set( _poViewportTextSafeArea->Res.x * 0.0750f, _poViewportTextSafeArea->Res.y * 0.0750f, 1.0f );
	paoTextSafeAreaVertsBuffer[ 1 ].Pos_MS.Set( _poViewportTextSafeArea->Res.x - ( _poViewportTextSafeArea->Res.x * 0.0750f ), _poViewportTextSafeArea->Res.y * 0.0750f, 1.0f );
	paoTextSafeAreaVertsBuffer[ 2 ].Pos_MS.Set( _poViewportTextSafeArea->Res.x * 0.0750f, _poViewportTextSafeArea->Res.y - ( _poViewportTextSafeArea->Res.y * 0.0750f ), 1.0f );
	paoTextSafeAreaVertsBuffer[ 3 ].Pos_MS.Set( _poViewportTextSafeArea->Res.x - ( _poViewportTextSafeArea->Res.x * 0.0750f ), _poViewportTextSafeArea->Res.y - ( _poViewportTextSafeArea->Res.y * 0.0750f ), 1.0f );
*/
	paoTextSafeAreaVertsBuffer[ 0 ].Pos_MS.Set( 640.0f * 0.0750f, 480.0f * 0.0750f, 1.0f );
	paoTextSafeAreaVertsBuffer[ 1 ].Pos_MS.Set( 640.0f - ( 640.0f * 0.0750f ), 480.0f * 0.0750f, 1.0f );
	paoTextSafeAreaVertsBuffer[ 2 ].Pos_MS.Set( 640.0f * 0.0750f, 480.0f - ( 480.0f * 0.0750f ), 1.0f );
	paoTextSafeAreaVertsBuffer[ 3 ].Pos_MS.Set( 640.0f - ( 640.0f * 0.0750f ), 480.0f - ( 480.0f * 0.0750f ), 1.0f );

	fdraw_Line( &( paoTextSafeAreaVertsBuffer[ 0 ] ), &( paoTextSafeAreaVertsBuffer[ 1 ] ) );
	fdraw_Line( &( paoTextSafeAreaVertsBuffer[ 2 ] ), &( paoTextSafeAreaVertsBuffer[ 3 ] ) );
	fdraw_Line( &( paoTextSafeAreaVertsBuffer[ 0 ] ), &( paoTextSafeAreaVertsBuffer[ 2 ] ) );
	fdraw_Line( &( paoTextSafeAreaVertsBuffer[ 1 ] ), &( paoTextSafeAreaVertsBuffer[ 3 ] ) );

	fvtxpool_ReturnArray( paoTextSafeAreaVertsBuffer );
	//
	////
#endif

_FText_Draw_Exit:

	//// Clean up.
	//
	frenderer_Pop();
	fviewport_SetActive( _poViewportPrevious );

	_bBlinkingAlphasNeedUpdate = TRUE;

	if( _poAreasHead )
	{
		_poTempArea = _poAreasHead;

		do
		{
			_poTempArea->bAreaDrawn = FALSE;
			_poTempArea->bBorderDrawn = FALSE;
			_poTempArea->fUnitCursorY = 0.0f;

			_poTempArea = _poTempArea->poAreaNext;

		} while( _poTempArea );
	}
	//
	////

#if _VERTS_USAGE_STATS
	//// Usage statistics.
	//
	if( _uStatVertsUsedCurrentDraw > _uStatVertsUsedMaxSingleDraw )
	{
		_uStatVertsUsedMaxSingleDraw = _uStatVertsUsedCurrentDraw;
	}
	_uStatVertsUsedTotal += _uStatVertsUsedCurrentDraw;
	++_uStatDrawsCalled;
	//
	////
#endif

	_nLettersUsed = 0;
} // ftext_Draw



void ftext_CalculateLetterCoordinates( FText_FTextPrintfLetter_t *pLetterStruct, 
								  f32 *pfUpperLeftX, f32 *pfUpperRightX, f32 *pfLowerLeftX, f32 *pfLowerRightX,
								  f32 *pfUpperY, f32 *pfLowerY ) {

	FDataFntFile_Font_t *poWorkingFont;

	//first, find the font this letter is associated with.
	poWorkingFont = _poFontsHead;

	do
	{
		if( poWorkingFont->oHandle == pLetterStruct->hFont )
		{
			break;
		}
		poWorkingFont = poWorkingFont->poFontNext;
	} while( poWorkingFont );

	//next, grab the letter structure within this font.
	FDataFntFile_Letter_t *pFontLetter = &poWorkingFont->paoFntLetters[ pLetterStruct->uFntLetterIdx ];

	f32	fFrameBufferRescaleFactor = _FRAMEBUFFER_X_SCALEFACTOR;
	if( pLetterStruct->nStats & _FTEXT_PRINTF_LETTERSTATS_OTOTEXELPIXELALIGNMENT ) {
		fFrameBufferRescaleFactor = 1.0f;
	}

	//now, calculate the 4 corners (minus italic shifts)
	f32 fX1, fX2, fY1, fY2; 
	// Upper Left
	fX1 = pLetterStruct->fX;
	fY1 = pLetterStruct->fY;

	// Lower Right
	fX2 = fX1 + ( pFontLetter->fWidth * pLetterStruct->fScale * fFrameBufferRescaleFactor );
	fY2 = fY1 + ( pFontLetter->fHeight * pLetterStruct->fScale );

	if( ( pLetterStruct->nStats & _FTEXT_PRINTF_LETTERSTATS_DEBUG ) || 
		( pLetterStruct->nStats & _FTEXT_PRINTF_LETTERSTATS_OTOTEXELPIXELALIGNMENT ) )
	{
		//fix the calculated lower corner
		// Lower Right
		fX2 = ( ( ( fX2 - (f32)( (s32)( fX2 ) ) ) > 0.5f ) ? ( (f32)( (s32)( fX2 ) ) + 1.5f ) : ( (f32)( (s32)( fX2 ) ) + 0.5f ) );
		fY2 = ( ( ( fY2 - (f32)( (s32)( fY2 ) ) ) > 0.5f ) ? ( (f32)( (s32)( fY2 ) ) + 1.5f ) : ( (f32)( (s32)( fY2 ) ) + 0.5f ) );
	}

	//now, apply the italic slant
	float fTopSlantOffset, fBottomSlantOffset;
	if( 0.0f < pLetterStruct->fItalicSlant ) {
		fTopSlantOffset = ( pLetterStruct->fItalicSlant * pFontLetter->fItalicPositiveSlantTop * fFrameBufferRescaleFactor );
		fBottomSlantOffset = ( pLetterStruct->fItalicSlant * pFontLetter->fItalicPositiveSlantBottom * fFrameBufferRescaleFactor );
	} else {
		fTopSlantOffset = ( pLetterStruct->fItalicSlant * pFontLetter->fItalicNegativeSlantTop * fFrameBufferRescaleFactor );
		fBottomSlantOffset = ( pLetterStruct->fItalicSlant * pFontLetter->fItalicNegativeSlantBottom * fFrameBufferRescaleFactor );
	}

	if( pfUpperLeftX ) {
		*pfUpperLeftX  = fX1 + fTopSlantOffset;
	}
	if( pfUpperRightX ) {
		*pfUpperRightX = fX2 + fTopSlantOffset;
	}
	if( pfLowerLeftX ) {
		*pfLowerLeftX  = fX1 + fBottomSlantOffset;
	}
	if( pfLowerRightX ) {
		*pfLowerRightX = fX2 + fBottomSlantOffset;
	}
	if( pfUpperY ) {
		*pfUpperY = fY1;
	}
	if( pfLowerY ) {
		*pfLowerY = fY2;
	}
}

u16 _FindFntLetterIndex( FDataFntFile_Font_t *pFont, u16 uCharCode ) {

	//First, check the LSB of the character..  That will point us in the right direction.
	u32 uBucketIndex = pFont->auLetterToBucketIndex[ uCharCode & 0xFF ];
	if( uBucketIndex == FDATA_FNT_LETTERBUCKET_EMPTY ) {
		return 0xFFFF;
	}

	u16 uRetIndex = 0xFFFF;

	//now, search through the bucket and see if we have a character
	u32 uBaseIndex = pFont->paoLetterBuckets[ uBucketIndex ].nBucketLetterBaseIndex;
	for( u32 i=0; i<pFont->paoLetterBuckets[ uBucketIndex ].nNumLettersInBucket; i++ ) {
		if( pFont->paoBucketLetters[ uBaseIndex + i ].uCharCode == uCharCode ) {
			uRetIndex = pFont->paoBucketLetters[ uBaseIndex + i ].uFntLetterIndex;
			break;
		}
	}

	return uRetIndex;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif // FTEXT_STUB_EVERYTHING_OUT

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
