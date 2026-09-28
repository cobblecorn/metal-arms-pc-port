//////////////////////////////////////////////////////////////////////////////////////
// fdx8movie2.cpp - PC/XBox implementation of Fang movie interface.
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
// 12/20/02 rfoushee    Created.
//////////////////////////////////////////////////////////////////////////////////////

// Set this define so that our fang types don't get hijacked by bink!
#define RAD_NO_LOWERCASE_TYPES

#include "fmovie2.h"
#include "fres.h"
#include "fdx8.h"
#include "bink.h"
#include "fclib.h"
#include "fdatastreaming.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// LOCAL DEFINES

#define _FMOVIE2_INTERNAL_MEMORY_BYTES ( 2*1024*1024 )

#if FANG_WINGC
// The PC port gives Bink a deep read-ahead: its background IO thread keeps this much of the movie ahead
// of playback, so a busy disk no longer stalls a movie mid-play (the frame loop waited on Bink's reads,
// hundreds of ms at a time). Bink's memory then comes from the heap rather than the consoles' 2 MB pool.
#define _FMOVIE2_PC_IO_BYTES ( 16*1024*1024 )
#endif

#if FANG_PLATFORM_WIN
#define _USE_STREAMING_MEMORY 0
#elif FANG_PLATFORM_XB
#define _USE_STREAMING_MEMORY 1
#endif


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// LOCAL STRUCTURES

typedef struct {
	char *pszName;
	char *pszPath;
} _MovieTableEntry_t;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// LOCAL VARIABLES

// Module.
static BOOL _bModuleStarted;
static BOOL _bModuleInstalled;

// Bink Variables
static _MovieTableEntry_t *_apMovieTable = NULL;
static u32 _uNumMovieTableEntries = 0;
static HBINK _hBink; //handle to the bink movie
static BOOL _bMoviePaused; //is the movie paused?
static BOOL _bMovieAudioEnabled = TRUE;
static f32 _fMovieVolume = 1.0f;
static BOOL _bNextFrameReady;
static u32 _nSkippedFrames;
static u32 _uPlayFlags = FMOVIE2_PLAYFLAGS_NONE;
#if FANG_WINGC
static IDirect3DSurface9 *_pMovieSurface;	// movie-sized decode target, stretched to the back buffer
#endif

#if !_USE_STREAMING_MEMORY
//windows management variables
static u32 _nInternalMemoryBase; //the base address to the internal memory
static u32 _nCurrentMemoryHead; //the address to the head of the available memory from the internal chunk
static u32 _nNumMemoryAllocations; //the number of chunks of memory Bink has currently taken.
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// LOCAL FUNCTIONS

static void* RADLINK _MovieAlloc ( U32 uNumBytes );
static void  RADLINK _MovieFree ( void* pMemToFree );

static BOOL _BuildMovieFilenameTable( cchar *pszBasePath, BOOL bScanSubDirs );
static BOOL _SearchDirectories ( cchar *pszBasePath, BOOL bScanSubDirs, 
							    _MovieTableEntry_t *apTableHead, u32 *puNumTableEntries ); 

#if FANG_PLATFORM_WIN
static BOOL _BltWinFrame( void );
#else
static BOOL _BltXBoxFrame( void );
#endif


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// PUBLIC INTERFACE FUNCTIONS

BOOL fmovie2_ModuleStartup( void ) {
	FASSERT_MSG( ( ! _bModuleStarted ), "[ FMOVIE2 ] Error: Module already initialized !!!" );

	_bModuleStarted   = TRUE;
	_bModuleInstalled = FALSE;

	if( Fang_ConfigDefs.pszMovie_BasePathName == NULL ) {
		goto _ExitWithError;
	}

	RADSetMemory( _MovieAlloc, _MovieFree ); //comment in if we want to track memory usage

#if !_USE_STREAMING_MEMORY
	_nInternalMemoryBase = 0;
	_nCurrentMemoryHead = 0;
	_nNumMemoryAllocations = 0;
#endif

	_apMovieTable = NULL;
	_uNumMovieTableEntries = 0;

	//BINK VARIABLES (these variables are protected by the Mutex Object)
	_hBink = NULL;
	_bNextFrameReady = FALSE;
	_bMoviePaused = FALSE;
	_nSkippedFrames = 0;

	_apMovieTable = NULL;

	//success
	return TRUE;

	//FAILURE
	_ExitWithError:
	DEVPRINTF( "[ FMOVIE2 ] INFO: System Failed to Startup -- Will continue to run without movie support!\n");
	return TRUE;
} //fmovie2_ModuleStartup


void fmovie2_ModuleShutdown( void ) {

	fmovie2_Uninstall();

	_apMovieTable = NULL;
	_uNumMovieTableEntries = 0;

#if !_USE_STREAMING_MEMORY
	_nInternalMemoryBase = 0;
	_nCurrentMemoryHead = 0;
	_nNumMemoryAllocations = 0;
#endif

	_bModuleStarted = FALSE;

} //fmovie2_ModuleShutdown


FMovie2_Error_e fmovie2_Install( void ) {

	FASSERT_MSG( _bModuleStarted,     "[ FMOVIE2 ] Error: System not started !!!" );
	FASSERT_MSG( ! _bModuleInstalled, "[ FMOVIE2 ] Error: System already installed !!!" );

	//set up sound support (if running windows)
	//sound support on XBOX is automatically supported.
#if FANG_PLATFORM_WIN
	//first, set up bink
	if( !BinkSoundUseDirectSound( 0 ) )
	{
		DEVPRINTF( "[ FMOVIE2 ] Error %u: BinkSoundUseDirectSound failed !!!\n", __LINE__ );
		goto _ExitWithError;
	}
#endif

	// Unload all of the default converters and then just load the ones we need.
#if FANG_PLATFORM_XB
	BinkUnloadConverter( BINKCONVERTERSALL );
	BinkLoadConverter( BINKSURFACE32 );
#endif

	//next, scan the directory system for movies
	BOOL bScanSubDirs = TRUE;
	if( ! _BuildMovieFilenameTable ( Fang_ConfigDefs.pszMovie_BasePathName, bScanSubDirs ) ) {
		//not a catastrophic issue, just bail out.
		goto _ExitWithError;
	}

	//check to see if we have any movies to play.  If we don't then don't grab memory!
	if( _uNumMovieTableEntries == 0 )
		return FMOVIE2_NO_ERROR;

#if !_USE_STREAMING_MEMORY
	//grab memory here for parceling out to bink as needed
	//get a resource frame pointer
	FResFrame_t ResFrame;
	ResFrame = fres_GetFrame();
	_nInternalMemoryBase = ( u32 ) fres_Alloc( _FMOVIE2_INTERNAL_MEMORY_BYTES );
	if( _nInternalMemoryBase == 0 ){
		goto _ExitWithError;
	}
	_nCurrentMemoryHead = _nInternalMemoryBase;
#endif


	_bModuleInstalled = TRUE;
	return FMOVIE2_NO_ERROR;

	//FAILURE
	_ExitWithError:
	DEVPRINTF( "[ FMOVIE2 ] INFO: System Failed to Initialize -- Will continue to run without movie support!\n");
    return FMOVIE2_NO_ERROR; //Right now, the movie system failing to install is not catastrophic    
} // fmovie2_Install


void fmovie2_Uninstall( void ) {
	//this routine is responsible for cleaning up the system
	//at uninstall time...

	if( ! _bModuleInstalled ) 
		return;

	fmovie2_Unload();

#if !_USE_STREAMING_MEMORY
	_nInternalMemoryBase = NULL;
	_nCurrentMemoryHead = NULL;
	_nNumMemoryAllocations = 0;
#endif

	_bModuleInstalled = FALSE;

} // fmovie2_Uninstall



BOOL fmovie2_IsInstalled( void ) {
	return _bModuleInstalled;
} // fmovie2_IsInstalled


void fmovie2_SetAudioEnabled( BOOL bEnabled ) {
	_bMovieAudioEnabled = bEnabled;
	if( _hBink ) {
		if( !_bMovieAudioEnabled ) {
			for( U32 uTrack = 0; uTrack < _hBink->NumTracks; ++uTrack )
				BinkSetVolume( _hBink, uTrack, 0 );
		} else {
			BinkSetVolume( _hBink, 0, (s32)( _fMovieVolume * 32768 ) );
		}
	}
}


void fmovie2_Play( cchar *pszFileName, f32 fVolume, cu32 uPlayFlags ) {

	//This routine starts the process to play movies and currently spews 
	//them to the backbuffer.

	_uPlayFlags = uPlayFlags;

	//A future version of this may accept a texture pointer which
	//overrides output to the frame buffer and writes the data to
	//the texture instead. 
#if FANG_PLATFORM_XB
	D3DSurface *pDX8BackBuffer;
#else
	IDirect3DSurface8 *pDX8BackBuffer;
#endif
	D3DSURFACE_DESC BackBufferSurfaceDesc;

	if( !_bModuleInstalled )
		return;

	if( _hBink ) {
		//we have a movie already loaded... Error out!
		DEVPRINTF( "[ FMOVIE2 ] Error : Attempting to Load a new Bink Movie with an Existing Movie still loaded!\n");
		return;
	}

	//search the MovieFilenameTable to see if this movie is in our table...
	_MovieTableEntry_t *pMovieEntry = NULL;
	for( u32 ui=0; ui<_uNumMovieTableEntries; ui++ ) {
		if( !fclib_stricmp( _apMovieTable[ ui ].pszName, pszFileName ) ) {
			//we found a filename match... record off this structure and break out!
			pMovieEntry = &_apMovieTable[ ui ];
			break;
		}
	}

	if( !pMovieEntry ) {
		DEVPRINTF( "[ FMOVIE2 ] Error : Couldn't find movie file %s in Movie Filename Table!\n", pszFileName );
		return;
	}

	//if we are here, we found this filename in the movie table, lets construct the filename and load it!
	char szFullMoviePathName[ MAX_PATH ];
	fclib_strcpy( szFullMoviePathName, pMovieEntry->pszPath );
	fclib_strcat( szFullMoviePathName, pMovieEntry->pszName );

	BOOL bFilenameLocalized = ffile_LocalizeFilename( szFullMoviePathName, FALSE ); //DO NOT RENAME FILE, JUST DETERMINE IF IT'S LOCALIZED

#if _USE_STREAMING_MEMORY
	FDS_StreamMgr.LockCache();
#endif

	u32 uFlags = 0;
	if( _uPlayFlags & FMOVIE2_PLAYFLAGS_LOADENTIREMOVIEINTOMEMORY )
		uFlags |= BINKPRELOADALL;

	// Next, check to see if we need to do any localization specific work
	if( bFilenameLocalized ) { 
		// We have to change the Bink track ID to the localized track ID, 
		// which happens to be the same ID as the Language Character itself.  (Spiffy, eh?)
		char cLanguageChar, cAudioLanguageChar;
		U32 nSoundTrack;
		ffile_GetLanguageChars( &cLanguageChar, &cAudioLanguageChar );
		nSoundTrack = (u32) cLanguageChar;
		BinkSetSoundTrack( 1, &nSoundTrack );

		// Next, let bink know that it needs to honor this reqest
		uFlags |= BINKSNDTRACK;
	}

#if FANG_WINGC
	BinkSetIOSize( _FMOVIE2_PC_IO_BYTES );
	uFlags |= BINKIOSIZE;
#endif

#if FANG_WINGC
	DWORD uOpenStart = GetTickCount();
#endif
	_hBink = BinkOpen( szFullMoviePathName, uFlags );
#if FANG_WINGC
	if( Fang_bPortDiag )
		DEVPRINTF( "PORT-BINK open-ms %s %lu\n", pszFileName, (unsigned long)( GetTickCount() - uOpenStart ) );
#endif
	if( !_hBink ) {
		//there was an error loading the bink file.
		DEVPRINTF( "[ FMOVIE2 ] Error : Couldn't load movie file %s\n", szFullMoviePathName );
		DEVPRINTF( "[ FMOVIE2 ] Error (cont) : %s\n", BinkGetError() );

#if _USE_STREAMING_MEMORY
		FDS_StreamMgr.UnlockCache();
#endif

		return;
	}
	// -no-audio skips Fang's audio system. Keep Bink's own playback clock active and mute its tracks
	// by volume; turning Bink sound off can leave its video clock waiting forever on a silent soundtrack.
	if( !_bMovieAudioEnabled ) {
		for( U32 uTrack = 0; uTrack < _hBink->NumTracks; ++uTrack )
			BinkSetVolume( _hBink, uTrack, 0 );
	}
	if( Fang_bPortDiag )
		DEVPRINTF( "PORT-BINK open %s size=%ux%u frames=%u tracks=%u sound=%d\n", pszFileName,
			_hBink->Width, _hBink->Height, _hBink->Frames, _hBink->NumTracks, _bMovieAudioEnabled );

	//make sure the movie is no larger than the full screen buffer
//	CFTexInst *pFullscreenTex = fsh_GetFullScrTexture();
//	FTexDef_t *pTexDef = pFullscreenTex->GetTexDef();
//	if( ( _hBink->Width > pTexDef->TexInfo.nTexelsAcross) || 
//	    (_hBink->Height > pTexDef->TexInfo.nTexelsDown ) ) {


	//Get the backbuffer for determining attributes and flags
	if( FAILED( FDX8_pDev->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pDX8BackBuffer ) ) ) {
		DEVPRINTF( "[ FMOVIE2 ] Error : Could not get DX8 Backbuffer!\n");
		fmovie2_Unload();
		return;
	}

	if( FAILED( pDX8BackBuffer->GetDesc( &BackBufferSurfaceDesc ) ) ) {
		DEVPRINTF( "[ FMOVIE2 ] Error : Could not get DX8 Backbuffer Description!\n");
        pDX8BackBuffer->Release();
		fmovie2_Unload();
		return;
	}
    pDX8BackBuffer->Release();

	if( ( _hBink->Width > BackBufferSurfaceDesc.Width ) || 
	    ( _hBink->Height > BackBufferSurfaceDesc.Height ) ) {
		DEVPRINTF( "[ FMOVIE2 ] Error : Movie %s larger than fullscreen texture surface.  Unloading Movie!\n", szFullMoviePathName );
		fmovie2_Unload();
		return;
	}

	//last thing to do is set the volume (clamp it to valid ranges first)...
	FMATH_CLAMPMIN( fVolume, 0.0f );
	FMATH_CLAMPMAX( fVolume, 1.0f );
	_fMovieVolume = fVolume;
	BinkSetVolume( _hBink, 0, _bMovieAudioEnabled ? (s32) ( fVolume * 32768 ) : 0 ); //Bink volume ranges from 0 to 32768

	//SUCCESS!
	_bMoviePaused = FALSE;
	_bNextFrameReady = TRUE; //have the Work() function decompress the first frame of the video
	_nSkippedFrames = 0;
}



void fmovie2_Pause( BOOL bPaused ) {
	if( !_bModuleInstalled )
		return;

	_bMoviePaused = bPaused;

	if( _hBink ) {
		BinkPause( _hBink, _bMoviePaused );	
	}
}

void fmovie2_Unload( void ) {

	if( !_bModuleInstalled )
		return;

	if( _hBink ) {
#if FANG_WINGC
		DWORD uCloseStart = GetTickCount();
#endif
		BinkClose( _hBink );
#if FANG_WINGC
		if( Fang_bPortDiag )
			DEVPRINTF( "PORT-BINK close-ms %lu\n", (unsigned long)( GetTickCount() - uCloseStart ) );
#endif
		DEVPRINTF( "[ FMOVIE2 ] Info : Unloading Bink Movie -- Num Skipped Frames = %d\n", _nSkippedFrames);
	}

#if _USE_STREAMING_MEMORY
	FDS_StreamMgr.UnlockCache();
#endif

	_hBink = 0;
	_bMoviePaused = FALSE;
	_bNextFrameReady = FALSE;
	_nSkippedFrames = 0;
#if FANG_WINGC
	if( _pMovieSurface ) {
		_pMovieSurface->Release();
		_pMovieSurface = NULL;
	}
#endif
}


FMovie2Status_e fmovie2_GetStatus( void ) {
	//This routine will tell the App what the status 
	//of the movie system is...
	if( !_bModuleInstalled )
		return FMOVIE2_STATUS_UNLOADED;

	FMovie2Status_e nRetVal = FMOVIE2_STATUS_PLAYING;
	if( !_hBink )
		nRetVal = FMOVIE2_STATUS_UNLOADED;
	else if( _bMoviePaused ) 
		nRetVal = FMOVIE2_STATUS_PAUSED;

	return nRetVal;
}


BOOL fmovie2_MovieExists( cchar* pszFileName ) {
	BOOL bRetVal = FALSE;
#if FANG_WINGC
	{
		// Movie audio through the game's DirectSound device rather than a second device of Bink's own
		// (two devices contending showed as clipped, stuttering cutscene audio). The device exists once
		// audio is installed, which is after this module installs, so hand it over here.
		extern void *fdx8audio_GetDirectSound( void );
		static void *_pBinkDS = NULL;
		void *pDS = fdx8audio_GetDirectSound();
		if( pDS && pDS != _pBinkDS && BinkSoundUseDirectSound( pDS ) ) {
			_pBinkDS = pDS;
		}
	}
#endif

	//search the MovieFilenameTable to see if this movie is in our table...
	_MovieTableEntry_t *pMovieEntry = NULL;
	for( u32 ui=0; ui<_uNumMovieTableEntries; ui++ ) {
		if( !fclib_stricmp( _apMovieTable[ ui ].pszName, pszFileName ) ) {
			//we found a filename match...
			bRetVal = TRUE;
			break;
		}
	}
	return bRetVal;
}



BOOL fmovie2_Draw( BOOL bBlockTillNextFrame ) {
	//this routine should get called by the game loop once a frame when movies are playing.
	// a) block until next frame movie frame is ready for display (unless paused or bBlockTillNextFrame == FALSE)
	// b) blit the current frame to the framebuffer
	// c) check to see if the last frame has been displayed.
	//    if so, terminate and unload the current movie, unless FMOVIE2_PLAYFLAGS_LOOPING is set.

	if( !_bModuleInstalled )
		return TRUE;

#if FANG_WINGC
	// Never spin until the next movie frame is due: show each new frame on the first display frame after
	// Bink says it's due and let the swap interval pace presentation. Spinning and then waiting for vsync
	// made the two clocks beat (uneven frame times) and held a CPU core that Bink's audio thread needed.
	bBlockTillNextFrame = FALSE;
#endif

	BOOL bUnloadMovie = FALSE;
	if( _hBink ) {

		//if we are here, we are ready to decompress the next frame...
		if( _bNextFrameReady && !_bMoviePaused )
			_nSkippedFrames += BinkDoFrame( _hBink );

		//now transfer the current frame image to the frame buffer
#if FANG_PLATFORM_WIN
		if( !_BltWinFrame() ) {
			fmovie2_Unload();
			return FALSE;
		}
#endif
#if FANG_PLATFORM_XB
		if( !_BltXBoxFrame() ) {
			fmovie2_Unload();
			return FALSE;
		}
#endif

		//check to see if we have played our last frame
		//if so, set the unload flag

		if( _bNextFrameReady && !_bMoviePaused ) {
			if ( !(_uPlayFlags & FMOVIE2_PLAYFLAGS_LOOPING ) && ( _hBink->FrameNum == _hBink->Frames ) ) 
				bUnloadMovie = TRUE;
			else
				BinkNextFrame( _hBink );
		}
		BOOL bWaiting = FALSE;
		while( ( bWaiting = BinkWait( _hBink ) ) && !_bMoviePaused && bBlockTillNextFrame ) { };

		_bNextFrameReady = !bWaiting;
	}

	//last thing to do is check to see if we need to unload the movie
	if( bUnloadMovie )
		fmovie2_Unload();

	return TRUE;
}

BOOL fmovie2_TimeToDraw( void ) {
	if( !_bModuleInstalled || !_hBink )
		return FALSE;

	BOOL bWaiting = BinkWait( _hBink );
	
	if( !_bNextFrameReady )
		_bNextFrameReady = !bWaiting;
	
	return _bNextFrameReady;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// PRIVATE IMPLEMENTATION FUNCTIONS

void* RADLINK _MovieAlloc ( U32 uNumBytes ) {

#if FANG_WINGC
	// the heap (see _FMOVIE2_PC_IO_BYTES); -1 lets Bink fall back to its own allocator
	void *pHeapMemory = malloc( uNumBytes );
	return pHeapMemory ? pHeapMemory : (void *)-1;
#endif

#if !_USE_STREAMING_MEMORY
	FASSERT( _nInternalMemoryBase != 0 );
	FASSERT( _nCurrentMemoryHead != 0 );

	//make sure we have enough memory to grant this request
	if( ( _nCurrentMemoryHead + uNumBytes ) > ( _nInternalMemoryBase + _FMOVIE2_INTERNAL_MEMORY_BYTES ) ) {
		//too much memory is being requested... we gotta abort this movie and notify !!!
		DEVPRINTF( "[ FMOVIE2 ] _MovieAlloc ERROR -- Requesting %d bytes, only have %d available\n",
			uNumBytes, _nInternalMemoryBase + _FMOVIE2_INTERNAL_MEMORY_BYTES - _nCurrentMemoryHead );
		return ( void* )-1;
	}

	//if we are here, then we need to grant a chunk of memory and update our head 
	void* pMemory = ( void* ) _nCurrentMemoryHead;
	_nCurrentMemoryHead  += uNumBytes;
	_nNumMemoryAllocations++;
#else 
	// I know this cast down below is really kind of a no-no, but Bink has it's 
	// own definition of what a u32 is, and it doesn't jive with fang's version
	// of a u32.  So, I do this to get around the problem
	void* pMemory = FDS_StreamMgr.AllocFromCache( uNumBytes );
#endif

	DEVPRINTF( "[ FMOVIE2 ] _MovieAlloc -- Address %X, %d bytes\n", pMemory, uNumBytes );
	return pMemory; 
}


void RADLINK _MovieFree ( void* pMemToFree ) {
#if FANG_WINGC
	free( pMemToFree );
	return;
#endif
#if !_USE_STREAMING_MEMORY
	//this is a hack memory tracking system which just tracks the NUMBER of memory allocations
	//made and then freed.  When the total number of allocations reaches zero, then
	//the _nCurrentMemoryHead value is reset back to the _nInternalMemoryBase value.
	FASSERT( _nNumMemoryAllocations > 0 );

	_nNumMemoryAllocations--;

	if(_nNumMemoryAllocations == 0)
        _nCurrentMemoryHead = _nInternalMemoryBase;
#endif

	DEVPRINTF( "[ FMOVIE2 ] _MovieFree -- %X\n", pMemToFree );
}


BOOL _BuildMovieFilenameTable( cchar *pszBasePath, BOOL bScanSubDirs ) {
	//This routine will scan the root path (and possibly subdirs) for bink files TWO TIMES.
	//1 - This first scan will return the number of valid files found.
	//2 - next, we will allocate the appropriate memory for the table, and then
	//3 - scan again, this time filling in the table data as we find the valid files...

	FResFrame_t ResFrame;

	//check to make sure table is not already created
	if ( _apMovieTable || _uNumMovieTableEntries ) {
		//we already have scanned for movie entries... too bad!
		DEVPRINTF( "[ FMOVIE2 ] fmovie2_InitMovieFilenameTable -- Already previously initialized Filename Table!" );
		return FALSE;
	}

	//if we are here, we have not yet scanned for movies
	// grab memory frames...
	ResFrame = fres_GetFrame();

	//make the first scan
    BOOL bSuccess = _SearchDirectories( pszBasePath, bScanSubDirs, NULL, &_uNumMovieTableEntries ); 
	if( !bSuccess ) { // we had a scan error, return FALSE
		return FALSE;
	}

	if( _uNumMovieTableEntries == 0 ) { //no movies found, just return out
		return TRUE;
	}

	//next, allocate the Table memory and zero out
	_apMovieTable = ( _MovieTableEntry_t* ) fres_AllocAndZero( sizeof( _MovieTableEntry_t ) * _uNumMovieTableEntries );
	if( _apMovieTable == NULL ) {
		DEVPRINTF( "[ FMOVIE2 ] fmovie2_InitMovieFilenameTable -- Not enough memory to create Filename Table!" );
		fres_ReleaseFrame( ResFrame );
		return FALSE;
	}

	//now, do a second scan, this time filling in the information in the table as we go
	bSuccess = _SearchDirectories( pszBasePath, bScanSubDirs, _apMovieTable, &_uNumMovieTableEntries );
	if( !bSuccess ) {
		fres_ReleaseFrame ( ResFrame );
		return FALSE;
	}

	return TRUE;
}



BOOL _SearchDirectories ( cchar *pszBasePath, BOOL bScanSubDirs, 
						  _MovieTableEntry_t *apTableHead, u32 *puNumTableEntries ) 
{ 
	//this routine will scan the base directory (and possibly subdirectories)
	//for bink files.  If valid apTableHead and puNumTableEntries variables are
	//passed in, then this routine will also allocate memory and fill out the table
	//entries for tables as the search progresses.

	*puNumTableEntries = 0;
	WIN32_FIND_DATAA FindData;
	HANDLE hFind;
	char szSearchBasePath[ MAX_PATH ];
	
	//first, copy the base path into the search base path local variable
	fclib_strcpy( szSearchBasePath, pszBasePath );
	//next, check to see if it has a trailing backslash (for search purposes)
	u32 uSearchBasePathLength = fclib_strlen( szSearchBasePath );
	if( szSearchBasePath[ uSearchBasePathLength - 1 ] != '\\' )
		fclib_strcat( szSearchBasePath, "\\" );

	//now, append the wildcard character to the search string as well
	fclib_strcat( szSearchBasePath, "*" );

	//now, start the directory search
	hFind = FindFirstFileA( szSearchBasePath, &FindData );
    if( INVALID_HANDLE_VALUE == hFind ) {
		//no files found
		return TRUE;
    }
    else {
        do {
			if( bScanSubDirs && ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {
				//we found a directory, make sure it's not the '.' or the ".." directory...
				if( fclib_strcmp( FindData.cFileName, "." ) && fclib_strcmp( FindData.cFileName, ".." ) ) {
					//recursively call this routine and search this directory
					char szSubDirFullPath[ MAX_PATH ];
					u32 uSubDirNumTableEntries = 0;

					//construct the full path for this subdirectory
					fclib_strcpy( szSubDirFullPath, szSearchBasePath );
					char* pszTrailingStar = &szSubDirFullPath[ fclib_strlen( szSubDirFullPath ) - 1 ];
					//now append the subdirectory name on top of the trailing star search wildcard
					fclib_strcpy( pszTrailingStar, FindData.cFileName );

					_MovieTableEntry_t *apSubDirTableHead = NULL;
					if( apTableHead )
						apSubDirTableHead = &apTableHead[ *puNumTableEntries ];
					_SearchDirectories( szSubDirFullPath, bScanSubDirs, apSubDirTableHead, &uSubDirNumTableEntries );
					*puNumTableEntries += uSubDirNumTableEntries;
				}
			}
			if( !( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {
				//this is a valid filename.  We COULD check the final 4
				//characters in the filename (the extention) for a .BIK
				//ending if it's desired... 
				//Right now, just add the filename to the list and make an 
				//internal rule that only Bink files can be in the directories 
				//we search.
				if( apTableHead ) {
					//memory needs to be allocated to store the filename in...
					u32 uFilenameLengthPlusNull = fclib_strlen( FindData.cFileName ) + 1;
					char* pszFileName = ( char* ) fres_Alloc( uFilenameLengthPlusNull );
					if( !pszFileName ) { //error allocating memory...
						DEVPRINTF( "[ FMOVIE2 ] _SearchDirectories -- Not enough memory to create Filename string!" );
				        FindClose( hFind );
						return FALSE;
					}
					fclib_strcpy ( pszFileName, FindData.cFileName );
					apTableHead[ *puNumTableEntries ].pszName = pszFileName;

					//now, construct the BASE PATH.  Put a null character in that
					//final trailing backslash for the next bit of work
					u32 uBasePathLengthPlusNull = fclib_strlen( szSearchBasePath );
					char* pszBasePath = ( char* ) fres_Alloc( uBasePathLengthPlusNull );
					if( !pszBasePath ) { //error allocating memory...
						DEVPRINTF( "[ FMOVIE2 ] _SearchDirectories -- Not enough memory to create BasePath string!" );
				        FindClose( hFind );
						return FALSE;
					}
					fclib_strncpy ( pszBasePath, szSearchBasePath, uBasePathLengthPlusNull );
					pszBasePath[ uBasePathLengthPlusNull - 1] = 0; //append the trailing null over the '*' char
					apTableHead[ *puNumTableEntries ].pszPath = pszBasePath;
				}
                //lastly, increment the number of table entries...
				(*puNumTableEntries)++;
			}
        } while( FindNextFileA( hFind, &FindData ) );

        // Close the find handle.
        FindClose( hFind );
    }

	return TRUE;
}



#if FANG_PLATFORM_WIN
BOOL _BltWinFrame( void ) {

	//This routine will lock the XBox Back Buffer and then copy
	//the new frame to the XBox Back Buffer

	//NOTE: THIS ROUTINE ASSUMES THE DATA MUTEX HAS ALREADY BEEN TAKEN!

	IDirect3DSurface8 *pDX8BackBuffer;
	D3DLOCKED_RECT LockedRect;
	DWORD dwBinkSurfaceType;
	D3DSURFACE_DESC BackBufferSurfaceDesc;

	//Get the backbuffer for determining attributes and flags
	//as well as Blt'ing on to.
	if( FAILED( FDX8_pDev->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pDX8BackBuffer ) ) ) {
		DEVPRINTF( "[ FMOVIE2 ] Error : Could not get DX8 Backbuffer!\n");
		return FALSE;
	}

#if FANG_WINGC
	// The port's surfaces come from the D3D8-on-D3D9 shim, which Bink cannot inspect through
	// BinkDX8SurfaceType (it called through a null method). The copy below is BINKSURFACE32,
	// so require a 32-bit back buffer instead.
	dwBinkSurfaceType = BINKSURFACE32;
#else
	dwBinkSurfaceType = BinkDX8SurfaceType( pDX8BackBuffer );
#endif
	if( FAILED( pDX8BackBuffer->GetDesc( &BackBufferSurfaceDesc ) ) ) {
		DEVPRINTF( "[ FMOVIE2 ] Error : Could not get DX8 Backbuffer Description!\n");
        pDX8BackBuffer->Release();
		return FALSE;
	}
#if FANG_WINGC
	if( ( BackBufferSurfaceDesc.Format != D3DFMT_X8R8G8B8 && BackBufferSurfaceDesc.Format != D3DFMT_A8R8G8B8 ) ||
		BackBufferSurfaceDesc.Width < _hBink->Width || BackBufferSurfaceDesc.Height < _hBink->Height ) {
		DEVPRINTF( "[ FMOVIE2 ] Error : Back buffer is not a 32-bit surface large enough for the movie.\n" );
		pDX8BackBuffer->Release();
		return FALSE;
	}

	// Decode into a movie-sized surface and stretch it over the largest 4:3 area of the back
	// buffer: GameCube movies are 512x448 frames meant for a 4:3 television. On any failure,
	// fall through to the original unscaled, centered copy.
	IDirect3DDevice9 *pDev9 = FDX8_pDev->GetD3D9Device();
	if( !_pMovieSurface && FAILED( pDev9->CreateOffscreenPlainSurface( _hBink->Width, _hBink->Height, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &_pMovieSurface, NULL ) ) ) {
		_pMovieSurface = NULL;
	}
	D3DLOCKED_RECT MovieRect;
	if( _pMovieSurface && SUCCEEDED( _pMovieSurface->LockRect( &MovieRect, NULL, 0 ) ) ) {
		BinkCopyToBuffer( _hBink, MovieRect.pBits, MovieRect.Pitch, _hBink->Height, 0, 0, BINKSURFACE32 | BINKCOPYALL );
		_pMovieSurface->UnlockRect();

		u32 uW = BackBufferSurfaceDesc.Width, uH = BackBufferSurfaceDesc.Height;
		if( uW * 3 > uH * 4 ) {
			uW = uH * 4 / 3;
		} else {
			uH = uW * 3 / 4;
		}
		RECT DestRect;
		DestRect.left = (LONG)( BackBufferSurfaceDesc.Width - uW ) / 2;
		DestRect.top = (LONG)( BackBufferSurfaceDesc.Height - uH ) / 2;
		DestRect.right = DestRect.left + (LONG)uW;
		DestRect.bottom = DestRect.top + (LONG)uH;
		if( SUCCEEDED( pDev9->StretchRect( _pMovieSurface, NULL, pDX8BackBuffer, &DestRect, D3DTEXF_LINEAR ) ) ) {
			pDX8BackBuffer->Release();
			return TRUE;
		}
	}
#endif

	//now, figure out the center position of the movie...
	u32 uX = ( BackBufferSurfaceDesc.Width - _hBink->Width ) / 2;
	u32 uY = ( BackBufferSurfaceDesc.Height - _hBink->Height ) / 2;

	if( FAILED( pDX8BackBuffer->LockRect( &LockedRect, NULL, NULL ) ) ) {
		DEVPRINTF( "[ FMOVIE2 ] Error : Failed Lock of Backbuffer!\n");
        pDX8BackBuffer->Release();
		return FALSE;
	}

	BinkCopyToBuffer( _hBink,
					  LockedRect.pBits,
					  LockedRect.Pitch,
					  _hBink->Height,
					  uX,uY,
					  BINKSURFACE32 | BINKCOPYALL );

	if( FAILED( pDX8BackBuffer->UnlockRect() ) ) {
		DEVPRINTF( "[ FMOVIE2 ] Error : Failed Unlock of Backbuffer!\n");
        pDX8BackBuffer->Release();
		return FALSE;
	}

	pDX8BackBuffer->Release(); //decrement our reference count to the back buffer
	return TRUE;
}
#endif


#if FANG_PLATFORM_XB
BOOL _BltXBoxFrame( void ) {

	//This routine will lock the XBox Back Buffer and then copy
	//the new frame to the XBox Back Buffer

	//NOTE: THIS ROUTINE ASSUMES THE DATA MUTEX HAS ALREADY BEEN TAKEN!

	D3DSurface *pDX8BackBuffer;
	D3DLOCKED_RECT LockedRect;
	DWORD dwBinkSurfaceType;
	D3DSURFACE_DESC BackBufferSurfaceDesc;

	//Get the backbuffer for determining attributes and flags,
	//as well as Blt'ing on to.
	if( FAILED( FDX8_pDev->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pDX8BackBuffer ) ) ) {
		DEVPRINTF( "[ FMOVIE2 ] Error : Could not get DX8 Backbuffer!\n");
		return FALSE;
	}
	dwBinkSurfaceType = BinkDX8SurfaceType( pDX8BackBuffer );
	if( FAILED( pDX8BackBuffer->GetDesc( &BackBufferSurfaceDesc ) ) ) {
		DEVPRINTF( "[ FMOVIE2 ] Error : Could not get DX8 Backbuffer Description!\n");
        pDX8BackBuffer->Release();
		return FALSE;
	}

	//now, figure out the center position of the movie...
	u32 uX = ( BackBufferSurfaceDesc.Width - _hBink->Width ) / 2;
	u32 uY = ( BackBufferSurfaceDesc.Height - _hBink->Height ) / 2;

	if( FAILED ( pDX8BackBuffer->LockRect( &LockedRect, NULL, D3DLOCK_TILED ) ) ) {
		DEVPRINTF( "[ FMOVIE2 ] Error : Failed Lock of Backbuffer!\n");
        pDX8BackBuffer->Release();
		return TRUE;
	}

	BinkCopyToBuffer( _hBink,
					  LockedRect.pBits,
					  LockedRect.Pitch,
					  _hBink->Height,
					  uX,uY,
					  BINKSURFACE32 | BINKCOPYALL );

	if( FAILED( pDX8BackBuffer->UnlockRect() ) ) {
		DEVPRINTF( "[ FMOVIE2 ] Error : Failed Unlock of Backbuffer!\n");
        pDX8BackBuffer->Release();
		return FALSE;
	}

	pDX8BackBuffer->Release(); //decrement our reference count to the back buffer
	return TRUE;
}
#endif
