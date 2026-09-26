//////////////////////////////////////////////////////////////////////////////////////
// FTextMon.h - text output monitor 
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

#ifndef _FTEXTMON_H_
#define _FTEXTMON_H_ 1

#include "fang.h"
#include "ftext.h"


class CFTextMonitor
{
//-----------------------------------------------------------------------------
// public functions
//-----------------------------------------------------------------------------
public:
	CFTextMonitor();
	BOOL Init( u32 uLines, u32 uMaxLineSize );	// inits a CFTextMonitor object, allocates mem for text storage
	void UnInit();								
	void Work( void );							// periodic work function for object instances
	void DisplayWork( void );					// periodic display function for object instances
	void ClearDisplay( void );					// clears all lines of text being displayed
	void SetDisplay( BOOL bDisplay );			// turn on/off the monitor display
	void SetPosition( f32 fUpperLeftX, f32 fUpperLeftY );	// position the display on the screen
	void SetSize( f32 fXSize, f32 fYSize );		// size the monitor display.  NOT CURRENTLy IMPLEMENTED.
	void SetLineDisplayTime( f32 fTime );		// set the time that each line will display before disappearing (0.0 = doesn't disappear)
	void SetFlashing( BOOL bFlashing );			// Turns on/off flashing of text on monitor display
	void SetFlashTime( f32 fTime );				// sets period of flashing text
	void SetColor( f32 fRed, f32 fGreen, f32 fBlue );	// set color of display text
	void SetFlashColor( f32 fRed, f32 fGreen, f32 fBlue );	// set color of flashing display text
	void AddLine( cchar *pszFormat, ... );		// sends a new line of text to the monitor display
	static void Pause( BOOL bPause );			// tells text monitor system to pause/unpause game.
	static BOOL IsPaused( void );				// returns TRUE if text monitor system requested to pause game.

	FINLINE BOOL IsDisplayOn( void ) const { return m_bDisplayMonitor; };

//-----------------------------------------------------------------------------
// private data
//-----------------------------------------------------------------------------
private:
	char *m_paszLineText;		// pointer to two dimensional array of text: [m_uMaxLines][m_uMaxLineSize]
	u32 m_uCurrentLineIndex;	// index into m_paszLineText of most recently added line
	u32 m_uMaxLines;			// max lines of text displayed at a time
	u32 m_uMaxLineSize;			// maximum number of chars per line of monitor text INCLUDING ZERO TERMINATOR.
	BOOL m_bDisplayMonitor;		// TRUE if monitor text should be displayed
	f32 m_fXPos;				// x position of upper-left corner of monitor "window"
	f32 m_fYPos;				// y position of upper-left corner of monitor "window"
	f32 m_fXSize;				// x size of monitor "window"
	f32 m_fYSize;				// y size of monitor "window"
	f32 m_fLineDisplayTime;		// time in seconds to display each line of text on monitor screen
	f32 *m_pafLineDisplayTimes;	// pointer to array containing remaining display time for each line of text
	f32 m_fFlashCounter;		// timer for flashing text
	f32 m_fFlashTime;			// period of flashing text
	BOOL m_bFlashing;			// text should flash on/off
	CFColorRGBA m_TextColor;	// text display color information
	CFColorRGBA m_FlashTextColor;	// flashing text display color information
	FTextAreaHandle_t m_hTextBox;	// handle for text printing
	BOOL m_bIsInitialized;

	static BOOL m_bPaused;		// TRUE if text monitor has been asked to pause the game	
};




#endif	// _FTEXTMON_H_
