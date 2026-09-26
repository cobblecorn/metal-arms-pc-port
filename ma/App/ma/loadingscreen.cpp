//////////////////////////////////////////////////////////////////////////////////////
// loadingscreen.cpp - Loading Screen class.
//
// Author: Russell Foushee     
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
// 01/11/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "loadingscreen.h"
#include "ftimer.h"
#include "fvid.h"
#include "fviewport.h"
#include "ftext.h"
#include "fmovie2.h"
#include "gameloop.h"
#include "launcher.h"
#include "fclib.h"

#define _DEBUG_LOADING_STATS 0

#if _DEBUG_LOADING_STATS
	static u32 _uProgressCallCount = 0;
	static CFTimer _CallbackTimer;
	static f32 _fLastFlipTime;
	static f32 _fLastFrameRateUpdate;
	static u32 _uNumCallsSinceUpdate;
	static f32 _fCurrentFrameRate;
	static char _szDebugString[64];
#endif

static BOOL8 _bInitialized = FALSE;
static u8 _nHeadingLen;
static u32 _nFlips;
static cwchar *_pwszHeading;


BOOL loadingscreen_Init( cchar *pszLoadingMovie, cwchar *pwszHeading/*=NULL*/ ) {

#if _DEBUG_LOADING_STATS
	fclib_strcpy( _szDebugString, "");
	_uProgressCallCount = 0;
	_CallbackTimer.Reset();
	_fLastFlipTime = 0.0f;
	_fLastFrameRateUpdate = 0.0f;
	_uNumCallsSinceUpdate = 0;
	_fCurrentFrameRate = 0.0f;
#endif

	_nFlips = 0;
	fresload_SetProgressCallback( loadingscreen_Update );

	fmovie2_Unload(); //stop a previously playing movie
	fmovie2_Play( pszLoadingMovie, 1.0f, 
		FMOVIE2_PLAYFLAGS_LOADENTIREMOVIEINTOMEMORY | FMOVIE2_PLAYFLAGS_LOOPING );

	_bInitialized = TRUE;
	_pwszHeading = pwszHeading;
	if( _pwszHeading ) {
		_nHeadingLen = fclib_wcslen( _pwszHeading );
	} else {
		_nHeadingLen = 0;
	}

	return TRUE;
}

void loadingscreen_Uninit( void ) {
	//Unregister fresload Progress callback here
	fmovie2_Unload();
	fresload_SetProgressCallback( NULL );
	_bInitialized = FALSE;
#if _DEBUG_LOADING_STATS
	DEVPRINTF( "level_Load(): progress callback called %d\n", _uProgressCallCount );
#endif
}



BOOL loadingscreen_Update( cchar *pszDebugString/*=NULL*/ ) {
	if( !_bInitialized )
		return TRUE;

	// Check to see if the reset button has been pressed
	gameloop_CheckForPlatformReset( TRUE ); // Reset immediately
	
#if _DEBUG_LOADING_STATS
	if(pszDebugString)
		fclib_strcpy( _szDebugString, pszDebugString);
	_uProgressCallCount++;
	//check the timer and see if a 60th of a second has elapsed...
	f32 fCurrentTimeElapsed = _CallbackTimer.SampleSeconds();
#endif

	if( fmovie2_TimeToDraw() ) {
		_nFlips++;

#if _DEBUG_LOADING_STATS
		_uNumCallsSinceUpdate++;
		int iGreyscaleColor = _nFlips % 256;
		float fGreyscaleColor = ( f32 ) ( _nFlips ) / 255;

		//see if it's time to update the framerate
		if( ( fCurrentTimeElapsed - _fLastFrameRateUpdate ) >= 0.5f ) {
			//calculate the frame rate of the progress callback
			_fCurrentFrameRate = ( f32 ) ( _uNumCallsSinceUpdate ) / ( fCurrentTimeElapsed - _fLastFrameRateUpdate );
			_fLastFrameRateUpdate = fCurrentTimeElapsed;
			_uNumCallsSinceUpdate = 0;
		}
#endif
		fvid_Swap();
		fvid_Begin();
		fviewport_SetActive( NULL );

#if FANG_PLATFORM_WIN
		//clear every frame to fix debug driver rainbow warrior technology
		fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0, 0, 0, 1.0f, 0 );
#else
		if( ( _nFlips < 3 ) || ( fmovie2_GetStatus() != FMOVIE2_STATUS_PLAYING ) ) { 
			//do clears for the first couple of frames or if the movie isn't playing
			fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0, 0, 0, 1.0f, 0 );
		}
#endif

		fmovie2_Draw( FALSE );

		// draw the loading text...
		launcher_DrawLoadingText();
		if( _pwszHeading ) {
			if( _nHeadingLen < 38 ) {
				// short string, print at 1.1 size	
				ftext_Printf( 0.10f, 0.07f, L"~C70707099~f1~w0~aL~s1.10%ls", _pwszHeading );
			} else {
				// longer string, print at 1.0 size	
				ftext_Printf( 0.10f, 0.07f, L"~C70707099~f1~w0~aL~s1.00%ls", _pwszHeading );	
			}
		}
		
#if _DEBUG_LOADING_STATS
		if( fclib_strlen( _szDebugString ) ) {
			ftext_Printf( 0.1f, 0.3f, "~f1~C92929299~w1~al~s0.69%s", 
				_szDebugString); 
		}
		ftext_Printf( 0.1f, 0.1f, "~f1~C92929299~w1~al~s0.69CallRate =  %.02f -- Num Flips = %d", 
			_fCurrentFrameRate, _nFlips ); 

		ftext_Printf( 0.68f, 0.67f, "~f1~C92929299~w1~al~s0.69Ver %d.%d.%d", fversion_GetToolMajorVer(), 
																			fversion_GetToolMinorVer(),
																			fversion_GetToolSubVer() );
		_fLastFlipTime = fCurrentTimeElapsed;
#endif
		fvid_End();
	}

	return TRUE;
}
