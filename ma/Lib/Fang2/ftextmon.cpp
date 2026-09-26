//////////////////////////////////////////////////////////////////////////////////////
// FTextMon.cpp - text output monitor 
//
// Author: Chris MacDonald
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
// 10/29/02 MacDonald		Created.
//////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Description:
// 
// The CFTextMonitor class allows the on-screen display of a text messages similar
// to a shell window or debug output window.  The most recent messages appear at
// the bottom, with older messages being moved up to make room and eventually
// disappearing of the top of the monitor "window"
//
// Usage requires that an object instance be initialized with the number of lines
// of text that should be displayed by the monitor and the maximum text line length.
//
// The display work function must be called every frame for every object instance
// in order for the text to appear.

// The add line function is called to send a line to the display.
//-----------------------------------------------------------------------------

#include <stdio.h>		// _vsnprintf on xb/win
#include "ftextmon.h"
#include "fres.h"
#include "fangtypes.h"	// _vsnprintf on gc
#include "ftext.h"
#include "floop.h"
#include "fclib.h"
#include "fviewport.h"

BOOL CFTextMonitor::m_bPaused;						// TRUE if text monitor has been asked to pause the game

//-----------------------------------------------------------------------------
// defines
//-----------------------------------------------------------------------------

#define _DEFAULT_X_POS		( 0.1f )
#define _DEFAULT_Y_POS		( 0.1f )
#define _DEFAULT_X_SIZE		( 0.9f )
#define _DEFAULT_Y_SIZE		( 0.9f )
#define _ROW_HEIGHT			( 0.02f )
#define _MAX_LINE_SIZE		( 512 )
#define _FLASH_TIME			( 1.0f )

//-----------------------------------------------------------------------------
// public functions
//-----------------------------------------------------------------------------

CFTextMonitor::CFTextMonitor() {
	UnInit();
}

//-----------------------------------------------------------------------------
// initialize a CFTextMonitor object instance.  Allocates memory for text storage.
// uLInes = number of text lines visible on the display.
// uMaxLineSize = max number of chars on a line, including zero terminator character
// returns TRUE if initialization is successful, FALSE otherwise.
BOOL CFTextMonitor::Init( u32 uLines, u32 uMaxLineSize )
{
	#if FANG_PLATFORM_DX
	FASSERT( !m_bIsInitialized );
	FASSERT( uLines > 0 );
	FASSERT( uMaxLineSize > 0 );
	FResFrame_t hFrame;
	u32 nIndex;
	FTextArea_t TextArea;

	if( uMaxLineSize < 2 || uMaxLineSize > _MAX_LINE_SIZE )
	{
		FASSERT_NOW;
		return FALSE;
	}

	hFrame = fres_GetFrame();
	FASSERT( hFrame );
	if( hFrame == NULL )
	{
		return FALSE;
	}
	// allocate storage for uLines of text of size uMaxLineSize
	m_paszLineText = (char*) fres_AllocAndZero( uLines * uMaxLineSize );
	FASSERT( m_paszLineText );
	if( m_paszLineText == NULL ) goto ExitWithError;

	m_uCurrentLineIndex = 0;				// index into m_paszLineText of most recently added line
	m_uMaxLines = uLines;					// max lines of text displayed at a time
	m_uMaxLineSize = uMaxLineSize;			// maximum number of chars per line of monitor text INCLUDING ZERO TERMINATOR.
	m_bDisplayMonitor = TRUE;				// TRUE if monitor text should be displayed
	m_fXPos = _DEFAULT_X_POS;				// x position of upper-left corner of monitor "window"
	m_fYPos = _DEFAULT_Y_POS;				// y position of upper-left corner of monitor "window"
	m_fXSize = _DEFAULT_X_SIZE;				// x size of monitor "window"
	m_fYSize = _DEFAULT_Y_SIZE;				// y size of monitor "window"
	m_fLineDisplayTime = 0.0f;				// time in seconds to display each line of text on monitor screen (0.0 = permanent)
	m_fFlashTime = _FLASH_TIME;
	m_fFlashCounter = 0.0f;
	m_bPaused = FALSE;
	m_bFlashing = FALSE;

	m_TextColor.Set( 1.0f, 1.0f, 1.0f, 1.0f );
	m_FlashTextColor.Set( 0.0f, 0.0f, 0.0f, 1.0f );

	// allocate array containing remaining display time for each line of text
	m_pafLineDisplayTimes = (f32*) fres_Alloc( uLines * sizeof( f32 ) );
	FASSERT( m_pafLineDisplayTimes );
	if( m_pafLineDisplayTimes == NULL )
	{

		goto ExitWithError;
	}

	for( nIndex = 0; nIndex < uLines; nIndex++ )
	{
		m_pafLineDisplayTimes[nIndex] = 0.0f;
	}

   	ftext_SetToDefaults( &TextArea );
	TextArea.bVisible = FALSE;
	TextArea.fUpperLeftX = m_fXPos;
	TextArea.fLowerRightX = m_fXPos + m_fXSize;
	TextArea.fUpperLeftY = m_fYPos;
	TextArea.fLowerRightY = m_fYPos + m_fYSize;
	TextArea.fNumberOfLines = (f32) uLines;
	TextArea.fBorderThicknessX = 0.0f;
	TextArea.fBorderThicknessY = 0.0f;
	TextArea.oColorForeground.SetColor( 1.0f, 1.0f, 0.0f );
	TextArea.oColorForeground.SetAlpha( 1.0f );
	TextArea.oColorBackground.SetColor( 0.0f, 0.0f, 0.0f );
	TextArea.oColorBackground.SetAlpha( 0.6f );
	TextArea.oColorBorder.Set( 0.1f, 0.1f, 1.0f, 1.0f );
	TextArea.ohFont = '1';
	TextArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;
	m_hTextBox = ftext_Create( &TextArea );
	FASSERT( m_hTextBox );

	if( m_hTextBox != NULL ) {
        m_bIsInitialized = TRUE;
		return TRUE;
	}

ExitWithError:
	fres_ReleaseFrame( hFrame );
	#endif
	return FALSE;
}

void CFTextMonitor::UnInit() {
	m_bIsInitialized = FALSE;
	m_pafLineDisplayTimes = NULL;
	m_hTextBox = NULL;
}

//-----------------------------------------------------------------------------
void CFTextMonitor::Work( void )
{
}

//-----------------------------------------------------------------------------
// This function displays the monitor text and must be called every frame for
// every monitor object instance for which text output is desired.
void CFTextMonitor::DisplayWork( void )
{
	#if FANG_PLATFORM_DX
	FASSERT( m_bIsInitialized );

	char *pszLineText;
	u32 uRow;
	u32 uTextLine;
	FTextArea_t *pTextArea;

	pTextArea = ftext_GetAttributes( m_hTextBox );
	if( !pTextArea )
	{
		return;
	}

	// don't display background box unless lines of text are printed below
	pTextArea->bVisible = FALSE;

	if( !m_bDisplayMonitor )
	{
		return;
	}


	if( m_bFlashing )
	{
		m_fFlashCounter -= FLoop_fRealPreviousLoopSecs;	// using "real" value since we want text to flash even if game is paused
		if( m_fFlashCounter < 0.0f )
		{
			m_fFlashCounter = m_fFlashTime;
		}
	}

	// Save the old viewport and set a full-screen viewport
	FViewport_t* pOldVP = fviewport_GetActive();
	fviewport_SetActive(FViewport_pDefaultOrtho);

	// print all lines
	uRow = 0;
	uTextLine = m_uCurrentLineIndex;
	do
	{
		// construct pointer to start of storage for the current text line.
		pszLineText = m_paszLineText;
		pszLineText += uTextLine * m_uMaxLineSize;

		if( m_fLineDisplayTime > 0.0f )
		{
			m_pafLineDisplayTimes[uTextLine] -= FLoop_fPreviousLoopSecs;
			if( m_pafLineDisplayTimes[uTextLine] < 0.0f )
			{
				m_pafLineDisplayTimes[uTextLine] = 0.0f;
			}
		}

		if( m_pafLineDisplayTimes[uTextLine] > 0.0f || m_fLineDisplayTime == 0.0f )
		{
			if( m_bFlashing && m_fFlashCounter > m_fFlashTime * 0.5f )
			{
				pTextArea->oColorForeground.SetColor( m_FlashTextColor );
			}
			else
			{
				pTextArea->oColorForeground.SetColor( m_TextColor );
			}

			// make background box visible
			pTextArea->bVisible = TRUE;

			ftext_Printf( m_hTextBox, "%s", pszLineText );
		}

		++uRow;
		++uTextLine;
		if( uTextLine >= m_uMaxLines )
		{
			uTextLine = 0;
		}
	}
	while( uTextLine != m_uCurrentLineIndex );

	// Restore the viewport
	fviewport_SetActive( pOldVP );

	#endif
}

//-----------------------------------------------------------------------------
// clears all lines of text being displayed.
void CFTextMonitor::ClearDisplay( void )
{
#if FANG_PLATFORM_DX
	FASSERT( m_bIsInitialized );
	u32 nIndex;

	for( nIndex = 0; nIndex < m_uMaxLines; nIndex++ )
	{
		m_pafLineDisplayTimes[nIndex] = 0.0f;
	}

	m_uCurrentLineIndex = 0;
#endif
}

//-----------------------------------------------------------------------------
// Turns on and off the monitor text display
// bDisplay = TRUE to turn display on, FALSE to turn off
void CFTextMonitor::SetDisplay( BOOL bDisplay )
{
	#if FANG_PLATFORM_DX
	FASSERT( m_bIsInitialized );
	m_bDisplayMonitor = !!bDisplay;

	if( !m_bDisplayMonitor )
	{
		FTextArea_t *pTextArea;
		pTextArea = ftext_GetAttributes( m_hTextBox );
		if( pTextArea )
		{
			// turn off background box immediately since work function may not be called
			// after disabling display
			pTextArea->bVisible = FALSE;
		}
	}
	#endif
}

//-----------------------------------------------------------------------------
// Sets the position of the upper-left corner of the monitor display.
// fUpperLeftX = normalized screen X position (0.0 to 1.0)
// fUpperLeftY = normalized screen y position (0.0 to 1.0)
void CFTextMonitor::SetPosition( f32 fUpperLeftX, f32 fUpperLeftY )
{
#if FANG_PLATFORM_DX
	FASSERT( m_bIsInitialized );
	FTextArea_t *pTextArea;
	pTextArea = ftext_GetAttributes( m_hTextBox );
	if( !pTextArea )
	{
		return;
	}

	FASSERT( fUpperLeftX >= 0.0f && fUpperLeftX <= 1.0f );
	FASSERT( fUpperLeftY >= 0.0f && fUpperLeftY <= 1.0f );

	m_fXPos = fUpperLeftX;
	m_fYPos = fUpperLeftY;

	pTextArea->fUpperLeftX = m_fXPos;
	pTextArea->fLowerRightX = m_fXPos + m_fXSize;

	pTextArea->fUpperLeftY = m_fYPos;
	pTextArea->fLowerRightY = m_fYPos + m_fYSize;
#endif
}

//-----------------------------------------------------------------------------
// Sets the size of the monitor display.
// NOTE: THIS FUNCTIONALITY NOT CURRENTLY IMPLEMENTED.
// fXSize = normalized x size of display (0.0 to 1.0)
// fYSize = normalized x size of display (0.0 to 1.0)
void CFTextMonitor::SetSize( f32 fXSize, f32 fYSize )
{
#if FANG_PLATFORM_DX
	FASSERT( m_bIsInitialized );
	FTextArea_t *pTextArea;
	pTextArea = ftext_GetAttributes( m_hTextBox );
	if( !pTextArea )
	{
		return;
	}

	FASSERT( fXSize >= 0.0f && fXSize <= 1.0f );
	FASSERT( fYSize >= 0.0f && fYSize <= 1.0f );

	m_fXSize = fXSize;
	m_fYSize = fYSize;

	pTextArea->fUpperLeftX = m_fXPos;
	pTextArea->fLowerRightX = m_fXPos + m_fXSize;

	pTextArea->fUpperLeftY = m_fYPos;
	pTextArea->fLowerRightY = m_fYPos + m_fYSize;
#endif
}

//-----------------------------------------------------------------------------
// sets the amount of time each line of text should display before disappearing.
// fTime = time to display in seconds
void CFTextMonitor::SetLineDisplayTime( f32 fTime )
{
	#if FANG_PLATFORM_DX
	FASSERT( m_bIsInitialized );
	FASSERT( fTime >= 0.0f );
	m_fLineDisplayTime = fTime;
	#endif
}

//-----------------------------------------------------------------------------
// Turns on/off flashing of text on monitor display
void CFTextMonitor::SetFlashing( BOOL bFlashing )
{
#if FANG_PLATFORM_DX
	FASSERT( m_bIsInitialized );
	m_bFlashing = !!bFlashing;
#endif
}

//-----------------------------------------------------------------------------
// Sets period of flashing of text on monitor display
void CFTextMonitor::SetFlashTime( f32 fTime )
{
#if FANG_PLATFORM_DX
	FASSERT( m_bIsInitialized );
	FMATH_CLAMPMIN( fTime, 1.0f / 60.0f );
	m_fFlashTime = fTime;
#endif
}

//-----------------------------------------------------------------------------
// sets color of display text.  Params are 0.0 to 1.0
void CFTextMonitor::SetColor( f32 fRed, f32 fGreen, f32 fBlue )
{
#if FANG_PLATFORM_DX
	FASSERT( m_bIsInitialized );
	FTextArea_t *pTextArea;
	pTextArea = ftext_GetAttributes( m_hTextBox );
	if( !pTextArea )
	{
		return;
	}

	FMATH_CLAMP( fRed, 0.0f, 1.0f );
	FMATH_CLAMP( fGreen, 0.0f, 1.0f );
	FMATH_CLAMP( fBlue, 0.0f, 1.0f );

	m_TextColor.SetColor( fRed, fGreen, fBlue );
	pTextArea->oColorForeground.SetColor( m_TextColor );
#endif
}

//-----------------------------------------------------------------------------
// sets color of flashing display text.  Params are 0.0 to 1.0
void CFTextMonitor::SetFlashColor( f32 fRed, f32 fGreen, f32 fBlue )
{
#if FANG_PLATFORM_DX
	FASSERT( m_bIsInitialized );
	FMATH_CLAMP( fRed, 0.0f, 1.0f );
	FMATH_CLAMP( fGreen, 0.0f, 1.0f );
	FMATH_CLAMP( fBlue, 0.0f, 1.0f );

	m_FlashTextColor.SetColor( fRed, fGreen, fBlue );
#endif
}


//-----------------------------------------------------------------------------
// adds a line of text to the monitor.  format is as for printf()
// if input line is too long for monitor line length, text will
// wrap to subsequent monitor lines
void CFTextMonitor::AddLine( cchar *pszFormat, ... )
{
	#if FANG_PLATFORM_DX
	FASSERT( m_bIsInitialized );
	FANG_VA_LIST args;
	FANG_VA_START( args, pszFormat );
	s32 nLineSize;
	s32 nIndex;
	char *pszLineText;
	char tempBuf[_MAX_LINE_SIZE];

	// expand formatted text string into ordinary text string
	_vsnprintf( tempBuf, _MAX_LINE_SIZE-1, pszFormat, args );
	tempBuf[_MAX_LINE_SIZE-1] = 0;

	// find length of expanded string
	nLineSize = fclib_strlen( tempBuf );
	nIndex = 0;

	do
	{
		// construct pointer to start of storage for the current text line.
		pszLineText = m_paszLineText;
		pszLineText += m_uCurrentLineIndex * m_uMaxLineSize;

		fclib_strncpy( pszLineText, &tempBuf[nIndex], m_uMaxLineSize-1 );

		// terminate last char of this line in case all available chars were filled
		pszLineText[m_uMaxLineSize-1] = 0;

		// set display time for this line
		m_pafLineDisplayTimes[m_uCurrentLineIndex] = m_fLineDisplayTime;

		// point to next monitor line 
		++m_uCurrentLineIndex;
		if( m_uCurrentLineIndex >= m_uMaxLines )
		{
			m_uCurrentLineIndex = 0;
		}

		// advance index into expanded string
		nIndex += m_uMaxLineSize-1;

		// loop to next monitor line if remaining part of expanded string 
		// didn't fit on current line
	} while( nIndex < nLineSize );

	FANG_VA_END( args );
	#endif
}

//-----------------------------------------------------------------------------
// tells text monitor system to pause/unpause game.
void CFTextMonitor::Pause( BOOL bPause )
{
#if FANG_PLATFORM_DX
	m_bPaused = !!bPause;

	floop_PauseGame( m_bPaused );
#endif

}

//-----------------------------------------------------------------------------
// returns TRUE if text monitor system requested to pause game.
BOOL CFTextMonitor::IsPaused( void )
{
#if FANG_PLATFORM_DX
	return m_bPaused;
#else
	return FALSE;
#endif
}
