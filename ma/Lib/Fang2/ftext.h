//////////////////////////////////////////////////////////////////////////////////////
// ftext.h
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

#ifndef _FTEXT_H_
#define _FTEXT_H_ 1

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define FTEXT_STUB_EVERYTHING_OUT	FALSE

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if( ! ( FTEXT_STUB_EVERYTHING_OUT ) )

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fang.h"
#include "fdata.h"
#include "fcolor.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef u32 FTextAreaHandle_t;

typedef enum
{
	FTEXT_NO_ERROR	=	 0,
	FTEXT_ERROR		=	-1

} FTextError_e;

typedef enum
{
	FTEXT_HORZ_ALIGN_LEFT =	 0,
	FTEXT_HORZ_ALIGN_CENTER,
	FTEXT_HORZ_ALIGN_RIGHT,
//	FTEXT_HORZ_ALIGN_JUSTIFY,

} FTextHorzAlign_e;

typedef struct
{
	f32 fAlpha;				// will be multiplied by the font, background, and border to allow fading of the entire group
	f32 fUpperLeftX, fUpperLeftY;// what is the coordinate of the upper left corner (in unit float screen pixels, the bottom row of pixels will be screenH/screenW)
	f32 fLowerRightX, fLowerRightY;// what is the coordinate of the lower right corner (in unit float screen pixels, the bottom row of pixels will be screenH/screenW)
	f32 fUVUpperLeftX, fUVUpperLeftY;// what is the upper left texture coordinates for the box texture, if set
	f32 fUVLowerRightX, fUVLowerRightY;// what is the lower right texture coordinates for the box texture, if set
	f32 fNumberOfLines;		// how many lines should be drawn in the box, this will affect the size of each line of text in the box
	f32 fBorderThicknessX, fBorderThicknessY;// what is the thickness of the border around the box
	f32 fLineSpacing;		// how much space should there be between lines of text in the box
	f32 fTabSize;			// how much space should a tab be for text drawn in the box
	CFColorRGBA oColorForeground;// the color of the text
	CFColorRGBA oColorBackground;// the color of the background of the box
	CFColorRGBA oColorBorder;	// the border color
	FDataFntFile_Handle_t ohFont;// what font should be used when drawing text inside the box
	FTextHorzAlign_e oHorzAlign;// what type of alignment should be used when rendering text in the box

	// There are a lot of text areas, so let's save a little bit of space
	BOOL8 bNoScale;			// If true, force the font to not be scaled with the area
	BOOL8 bVisible;			// should the background box be drawn (doesn't matter if there is text in it or not)?
	BOOL8 bDrawGuides;		// should horizontal draw guides be drawn (ala looseleaf paper style)?
	BOOL8 bFixedWidthFont;	// should the associated font be drawn fixed width or not?
	BOOL8 bItalicFont;		// should the associated font be drawn italicized?

// NOT CURRENTLY SUPPORTED
	BOOL8 bClipFont; // FALSE: Cull (pop text), default. TRUE: Clip (scroll text).

} FTextArea_t;


enum { //bitfields describing various character options
	_FTEXT_PRINTF_LETTERSTATS_FIXEDWIDTH				= 0x01, //this character is printed with a fixed width character, warrants special width calculations
	_FTEXT_PRINTF_LETTERSTATS_OTOTEXELPIXELALIGNMENT	= 0x02, //this character is supposed to maintain a one to one pixel texel alignment
	_FTEXT_PRINTF_LETTERSTATS_DEBUG						= 0x04, //this character is printed through a debug

	_FTEXT_PRINTF_LETTERSTATS_NONE						= 0x00
};


struct FText_FTextPrintfLetter_t
{
	f32 fX;							// X Location of upper left corner of Character (BEFORE ITALIC SLANT APPLIED!)
	f32 fY;							// Y Location of upper left corner of Character
	f32 fScale;						// Scale factor to apply to character
	f32 fItalicSlant;				// ItalicSlant factor to apply to character
	CFColorPackedRGBA oColor;		// Color of the character
	FDataFntFile_Handle_t hFont;	// Handle to the font used for this letter
	u16 uFntLetterIdx;				// LetterIndex of a FDataFntFile_Letter_t structure;
	u8 nStats;						// Stats about how to rasterize this letter: See _FTEXT_PRINTF_* for info
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HERE ARE THE CURRENT ESCAPE CODES: (codes are not caps-sensitive, except the parameter of ~F.)
//
// ~AL - set the alignment to left justified.
// ~AR - set the alignment to right justified.
// ~AC - set the alignment to center justified.
// ~B0 thru ~B9 and ~BA - set the blinking rate to predefined motifs list below.
//			0: always on (default)
//			1: always off
// 			2: 1/8 sec on, 1/8 sec off
// 			3: 3/16 sec on, 1/16 sec off
// 			4: 1/4 sec on, 1/4 sec off
// 			5: 3/8 sec on, 1/8 sec off
// 			6: 3/8 sec on, 3/8 sec off
// 			7: 9/16 sec on, 3/16 sec off
// 			8: 1/2 sec on, 1/2 sec off
// 			9: 3/4 sec on, 1/4 sec off
//			A: cos based fading motif			
// ~CRRGGBBAA - set the current color (each value is on a 00 to 99 scale. 99 is 100%).
// ~FX - set the current font (X is an FDataFntFile_Handle_t, for example, '1', '2', 'a', 'A', etc).
// ~S1.00 - set the font scale value (for example, 1.00: natural size, 1.25: 25% larger than natural size, 0.85: 15% smaller than natural size, etc).
// ~T+XX thru ~T-XX - set the tilt, in pixels (relative to the font's natural size), when using italic feature (XX is a number from 00 to 99).
// ~W0 or ~W1 - turns fixed width OFF or ON.
// ~I0 or ~I1 - turns italic feature OFF or ON.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Module
extern BOOL ftext_ModuleStartup( void );
extern void ftext_ModuleShutdown( void );

extern FTextError_e ftext_Install( void );
extern void ftext_Uninstall( void );
extern BOOL ftext_IsInstalled( void );

extern void ftext_ResetBlinkTimers( void );

// Areas
extern void ftext_SetToDefaults( FTextArea_t *poArea );
extern FTextAreaHandle_t ftext_Create( const FTextArea_t *poArea, cchar *pszName = NULL ); // Handle 0: Error / Invalid.
extern FTextArea_t *ftext_GetAttributes( const FTextAreaHandle_t ohArea );
extern void ftext_SetAttributes( const FTextAreaHandle_t ohArea, const FTextArea_t *poArea );

//ANSI CHARACTER INTERFACES
extern void ftext_Printf( const FTextAreaHandle_t ohArea, cchar *pszFormat, ... );
extern void ftext_Printf( const f32 fX, const f32 fY, cchar *pszFormat, ... );
extern void ftext_Printf( const FTextAreaHandle_t ohArea, const f32 fX, const f32 fY, cchar *pszFormat, ... );

extern void ftext_PrintString( const FTextAreaHandle_t ohArea, cchar *pszString );
extern void ftext_PrintString( const f32 fX, const f32 fY, cchar *pszString );
extern void ftext_PrintString( const FTextAreaHandle_t ohArea, const f32 fX, const f32 fY, cchar *pszString );


//UNICODE CHARACTER INTERFACES
extern void ftext_Printf( const FTextAreaHandle_t ohArea, cwchar *pszWFormat, ... );
extern void ftext_Printf( const f32 fX, const f32 fY, cwchar *pszWFormat, ... );
extern void ftext_Printf( const FTextAreaHandle_t ohArea, const f32 fX, const f32 fY, cwchar *pszWFormat, ... );

extern void ftext_PrintString( const FTextAreaHandle_t ohArea, cwchar *pszWString );
extern void ftext_PrintString( const f32 fX, const f32 fY, cwchar *pszWString );
extern void ftext_PrintString( const FTextAreaHandle_t ohArea, const f32 fX, const f32 fY, cwchar *pszWString );

// STARICH - THIS DOESN'T WORK YET BECAUSE OF THE WAY THE FONTS ARE RENDERED, BUT
//			 THE GROUNDWORK IS LAID FOR USER SPECIFIED Z VALUES, SO I DIDN'T WANT TO RIP THIS OUT.
//			 THESE FUNCTIONS WILL RENDER THE SAME AS THE REGULAR VERSION FOR NOW. :-(
// allows strings to be Z pushed into the screen so that you can write ontop of other text.
// fZ must be 0.0f or greater.
extern void ftext_Printf_ZPush( f32 fX, f32 fY, f32 fZ, cchar *pszFormat, ... );
extern void ftext_PrintString_ZPush( f32 fX, f32 fY, f32 fZ, cchar *pszString );

// Default font, default area, default scale 1 to 1.
extern void ftext_DebugPrintf( const f32 fX, const f32 fY, cchar *pszFormat, ... );
extern void ftext_DebugPrintString( const f32 fX, const f32 fY, cchar *pszString );
extern void ftext_DebugPrintf( const f32 fX, const f32 fY, cwchar *pszFormat, ... );
extern void ftext_DebugPrintString( const f32 fX, const f32 fY, cwchar *pszString );

// Fonts
extern FTextError_e ftext_Load( FDataFntFile_Handle_t ohFont, cchar *pszName ); // Handle 0: Error / Invalid.
extern BOOL ftext_IsFontLoaded( cchar *pszName );
extern f32 ftext_GetLetterSpacing( const FDataFntFile_Handle_t ohFont );
extern f32 ftext_GetWordSpacing( const FDataFntFile_Handle_t ohFont );
extern f32 ftext_GetItalicSlant( const FDataFntFile_Handle_t ohFont );
extern void ftext_SetLetterSpacing( const FDataFntFile_Handle_t ohFont, const f32 fLetterSpacing );
extern void ftext_SetWordSpacing( const FDataFntFile_Handle_t ohFont, const f32 fWordSpacing );
extern void ftext_SetItalicSlant( const FDataFntFile_Handle_t ohFont, const f32 fItalicSlant );

// Output
extern void ftext_ClearAllPending( void );
extern void ftext_Draw( void );

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define FTEXT_DEFAULT_AREA_HANDLE					( 0 )
#define FTEXT_SET_DEBUG_COLOR( fR, fG, fB, fA )		ftext_GetAttributes( FTEXT_DEFAULT_AREA_HANDLE )->oColorForeground.Set( fR, fG, fB, fA )
#define FTEXT_SET_DEBUG_INTENSITY( fUnitIntensity )	ftext_GetAttributes( FTEXT_DEFAULT_AREA_HANDLE )->oColorForeground.Set( fUnitIntensity )

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif // FTEXT_STUB_EVERYTHING_OUT

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif // _FTEXT_H_

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
