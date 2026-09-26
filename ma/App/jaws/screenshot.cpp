//////////////////////////////////////////////////////////////////////////////////////
// screenshot.cpp - 
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
// 10/08/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "screenshot.h"
#include "fvid.h"
#include "screengrab.h"
#if FANG_PLATFORM_WIN
	#include "dx/fdx8vid.h"
#endif

//====================
// private definitions

#define _SNAP					0x1
#define _SINGLE_SHOT			0x2
#define _MULTI_SHOT				0x4
#define _KEY_DOWN				0x8

//=================
// public variables

//==================
// private variables

// screenshot vars
static u32 _nScreenshotFlag = _SINGLE_SHOT;// default to single shot
static BOOL _bScreenShotsSystemOK = FALSE;
static ScreenGrabOutputType_e _nType = SCREEN_GRAB_OUTPUT_TYPE_TGA;// default to tga

//===================
// private prototypes

//=================
// public functions

void screenshot_Init( cchar *pszScreenShotDir, BOOL bStartSnappingSeriesOnFrame1/*=TRUE*/ ) {

	_nScreenshotFlag &= ~_SNAP;
	if( FVid_Mode.nFlags & FVID_MODEFLAG_WINDOWED ) {
		// screenshots don't work in windowed mode
		_bScreenShotsSystemOK = FALSE;
	} else {
		_bScreenShotsSystemOK = screengrab_Init( pszScreenShotDir );
		if( bStartSnappingSeriesOnFrame1 ) {
			// if we are in multi mode at the begining, then make sure the shots start with frame 1
			if( _nScreenshotFlag & _MULTI_SHOT ) {
				_nScreenshotFlag |= _SNAP;
				_nScreenshotFlag |= _KEY_DOWN;
			}
		}
	}
}

void screenshot_Shutdown( void ) {
	screengrab_Shutdown();
	_bScreenShotsSystemOK = FALSE;
}

void screenshot_Work( void ) {
	
	if( _bScreenShotsSystemOK ) {
		// see if windows says that the space key is down
#if FANG_PLATFORM_WIN
		BOOL bKeyDown = fdx8vid_HaveFocus() && ( GetAsyncKeyState( VK_SPACE ) < 0 );
#else
		BOOL bKeyDown = FALSE;// ADD SOME WAY OF CLICKING A SCREENSHOT ON OTHER PLATFORMS	
#endif
		if( bKeyDown ) {
			// only process the first frame when the key is down, then wait for the next press
			if( (_nScreenshotFlag & _KEY_DOWN) == 0 ) {
				if( _nScreenshotFlag & _SNAP ) {
					// end the capture (only will happen in multi mode)
					_nScreenshotFlag &= ~_SNAP;
				} else {
					// start a capture 
					_nScreenshotFlag |= _SNAP;
				}
			}
			// mark that the key is down
			_nScreenshotFlag |= _KEY_DOWN;
		} else {
			// mark that the key is not down
			_nScreenshotFlag &= ~_KEY_DOWN;
		}
		if( _nScreenshotFlag & _SNAP ) {
			if( _nScreenshotFlag & _SINGLE_SHOT ) {
				screengrab_CaptureTheScreen( TRUE, _nType );
				_nScreenshotFlag &= ~_SNAP;				
			} else {
				screengrab_CaptureTheScreen( FALSE, _nType );				
			}
		}
	}
}

// if bSingle is TRUE then the screenshot mode will be put into single frame capture mode
// if FALSE, the screenshot mode will be put into multi mode
// NOTE: changing the screenshot mode will clear any mode that may be in effect 
// aka if there is a multi currently going, it will be stopped and a new one started the 
// next time F12 is pressed
void screenshot_SetScreenShotMode( BOOL bSingle ) {
	if( bSingle ) {
		_nScreenshotFlag = _SINGLE_SHOT;
	} else {
		_nScreenshotFlag = _MULTI_SHOT;
	}
}

// returns TRUE if in Single shot mode, FALSE if in Multi shot mode
BOOL screenshot_GetScreenShotMode( void ) {
	if( _nScreenshotFlag & _SINGLE_SHOT ) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void screenshot_SetScreenShotType( ScreenShotOutputType_e nScreenShotType ) {

	switch( nScreenShotType ) {

	case SCREENSHOT_OUTPUT_TYPE_TGA:
		_nType = SCREEN_GRAB_OUTPUT_TYPE_TGA;		
		break;

	case SCREENSHOT_OUTPUT_TYPE_JPEG:
		_nType = SCREEN_GRAB_OUTPUT_TYPE_JPEG;
		break;
	}
}

ScreenShotOutputType_e screenshot_GetScreenShotType( void ) {

	switch( _nType ) {
		
	case SCREEN_GRAB_OUTPUT_TYPE_TGA:
		return SCREENSHOT_OUTPUT_TYPE_TGA;

	case SCREEN_GRAB_OUTPUT_TYPE_JPEG:
		return SCREENSHOT_OUTPUT_TYPE_JPEG;
	}
	// default to tga
	return SCREENSHOT_OUTPUT_TYPE_TGA;
}

//==================
// private functions



