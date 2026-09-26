//////////////////////////////////////////////////////////////////////////////////////
// fgcmovie2.cpp - GameCube implementation of Fang movie interface.
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
// 12/26/02 rfoushee    Created.
//////////////////////////////////////////////////////////////////////////////////////

// Set this define so that our fang types don't get hijacked by bink!
#define RAD_NO_LOWERCASE_TYPES

#include "fmovie2.h"
#include "fgcvid.h"
#include "fgcfile.h"
#include "fviewport.h"
#include "frenderer.h"
#include "fdraw.h"
#include "fclib.h"
#include "fsh.h"
#include "fres.h"
#include "bink.h"
#include "radmem.h"
#include "fvis.h"
#include "fdatastreaming.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// DEFINES
#define _MAX_PATH 255


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
static BOOL _bNextFrameReady;
static u32 _nSkippedFrames;
static u32 _uPlayFlags = FMOVIE2_PLAYFLAGS_NONE;


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// LOCAL FUNCTIONS

static void* RADLINK _MovieAlloc ( u32 uNumBytes );
static void  RADLINK _MovieFree ( void* pMemToFree );

static BOOL _BuildMovieFilenameTable( cchar *pszBasePath, BOOL bScanSubDirs );
static BOOL _SearchDirectories ( cchar *pszBasePath, BOOL bScanSubDirs, 
							    _MovieTableEntry_t *apTableHead, u32 *puNumTableEntries ); 
static BOOL _BltGCFrame( void );
static BOOL _BltGCTextureAndRender( void );


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// PUBLIC INTERFACE FUNCTIONS

BOOL fmovie2_ModuleStartup( void ) {
	FASSERT_MSG( ( ! _bModuleStarted ), "[ FMOVIE2 ] Error: Module already initialized !!!" );

	_bModuleStarted   = TRUE;
	_bModuleInstalled = FALSE;

	if( Fang_ConfigDefs.pszMovie_BasePathName == NULL ) {
		goto _ExitWithError;
	}

	RADSetMemory( _MovieAlloc, _MovieFree );

	//BINK VARIABLES (these variables are protected by the Mutex Object)
	_hBink = NULL;
	_bNextFrameReady = FALSE;
	_bMoviePaused = FALSE;
	_nSkippedFrames = 0;

	//success
	return TRUE;

	//FAILURE
	_ExitWithError:
	DEVPRINTF( "[ FMOVIE2 ] INFO: System Failed to Startup -- Will continue to run without movie support!\n");
    return TRUE; //Right now, the movie system failing to install is not catastrophic    


} //fmovie2_ModuleStartup



void fmovie2_ModuleShutdown( void ) {

	fmovie2_Uninstall();

	_apMovieTable = NULL;
	_uNumMovieTableEntries = 0;

	_bModuleStarted = FALSE;

} //fmovie2_ModuleShutdown



FMovie2_Error_e fmovie2_Install( void ) {

	FASSERT_MSG( _bModuleStarted,     "[ FMOVIE2 ] Error: System not started !!!" );
	FASSERT_MSG( ! _bModuleInstalled, "[ FMOVIE2 ] Error: System already installed !!!" );


	//first, set up bink
	if( !BinkSoundUseMusyX() )
	{
		DEVPRINTF( "[ FMOVIE2 ] Error %u: BinkSoundUseDirectSound failed !!!\n", __LINE__ );
		return FMOVIE2_NO_ERROR;
	}

	//next, scan the directory system for movies
	BOOL bScanSubDirs = TRUE;
	if( ! _BuildMovieFilenameTable ( Fang_ConfigDefs.pszMovie_BasePathName, bScanSubDirs ) ) {
		//not a catastrophic issue, just bail out.
		return FMOVIE2_NO_ERROR;
	}


	//check to see if we have any movies to play.  If we don't then don't register installed!
	if( _uNumMovieTableEntries == 0 )
		return FMOVIE2_NO_ERROR;

	_bModuleInstalled = TRUE;

	return FMOVIE2_NO_ERROR;
} // fmovie2_Install


void fmovie2_Uninstall( void ) {
	//this routine is responsible for cleaning up the system
	//at uninstall time...

	if( ! _bModuleInstalled ) 
		return;

	fmovie2_Unload();

	_bModuleInstalled = FALSE;

} // fmovie2_Uninstall



BOOL fmovie2_IsInstalled( void ) {
	return _bModuleInstalled;
} // fmovie2_IsInstalled


void fmovie2_Play( cchar *pszFileName, f32 fVolume, cu32 uPlayFlags ) {
	//This routine starts the process to play movies
	
	_uPlayFlags = uPlayFlags;

	

	//A future version of this may accept a texture pointer which
	//overrides output to the frame buffer and writes the data to
	//the texture instead. 
	
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
	char szFullMoviePathName[ _MAX_PATH ];
	fclib_strcpy( szFullMoviePathName, pMovieEntry->pszPath );
	fclib_strcat( szFullMoviePathName, pMovieEntry->pszName );

	BOOL bFilenameLocalized = ffile_LocalizeFilename( szFullMoviePathName, FALSE ); //DO NOT RENAME FILE, JUST DETERMINE IF IT'S LOCALIZED

	FDS_StreamMgr.LockCache();

	u32 uFlags = 0;
	if( _uPlayFlags & FMOVIE2_PLAYFLAGS_LOADENTIREMOVIEINTOMEMORY )
		uFlags |= BINKPRELOADALL;

	// Next, check to see if we need to do any localization specific work
	if( bFilenameLocalized ) { 
		// We have to change the Bink track ID to the localized track ID, 
		// which happens to be the same ID as the Language Character itself.  (Spiffy, eh?)
		char cLanguageChar, cAudioLanguageChar;
		u32 nSoundTrack;
		ffile_GetLanguageChars( &cLanguageChar, &cAudioLanguageChar );
		nSoundTrack = (u32) cLanguageChar;
		BinkSetSoundTrack( 1, &nSoundTrack );

		// Next, let bink know that it needs to honor this reqest
		uFlags |= BINKSNDTRACK;
	}

	// This will loop, trying to open the bink file until the file has
	// tried to be opened without a major DVD error
	do {
		_hBink = BinkOpen( szFullMoviePathName, uFlags );
	} while( fgcfile_HandleDVDErrors( TRUE, TRUE ) );
	
	if( !_hBink ) {
		//there was an error loading the bink file.
		DEVPRINTF( "[ FMOVIE2 ] Error : Couldn't load movie file %s\n", szFullMoviePathName );
		DEVPRINTF( "[ FMOVIE2 ] Error (cont) : %s\n", BinkGetError() );
		FDS_StreamMgr.UnlockCache();
		return;
	}

	//make sure the movie is no larger than the full screen buffer
	CFTexInst *pFullscreenTex = fsh_GetFullScrTexture();
	FTexDef_t *pTexDef = pFullscreenTex->GetTexDef();
	if( ( _hBink->Width > pTexDef->TexInfo.nTexelsAcross ) || 
	    ( _hBink->Height > pTexDef->TexInfo.nTexelsDown ) ) {
	    
		DEVPRINTF( "[ FMOVIE2 ] Error : Movie %s larger than fullscreen texture surface.  Unloading Movie!\n", szFullMoviePathName );
		fmovie2_Unload();
		return;
	}

	//last thing to do is set the volume (clamp it to valid ranges first)...
	FMATH_CLAMPMIN( fVolume, 0.0f );
	FMATH_CLAMPMAX( fVolume, 1.0f );
	BinkSetVolume( _hBink, 0, (s32) ( fVolume * 32768 ) ); //Bink volume ranges from 0 to 32768
	
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
		BinkClose( _hBink );
		DEVPRINTF( "[ FMOVIE2 ] Info : Unloading Bink Movie -- Num Skipped Frames = %d\n", _nSkippedFrames);
	}

	FDS_StreamMgr.UnlockCache();
	
	_hBink = 0;
	_bMoviePaused = FALSE;
	_bNextFrameReady = FALSE;
	_nSkippedFrames = 0;
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



BOOL fmovie2_TimeToDraw( void ) {
	if( !_bModuleInstalled || !_hBink )
		return FALSE;

	fgcfile_HandleDVDErrors( TRUE, TRUE ); 
	BOOL bWaiting = BinkWait( _hBink );
	
	if( !_bNextFrameReady )
		_bNextFrameReady = !bWaiting;
	
	return _bNextFrameReady;
}




BOOL fmovie2_Draw( BOOL bBlockTillNextFrame ) {
	//this routine should get called by the game loop once a frame when movies are playing.
	// a) block until next frame movie frame is ready for display (unless paused or bBlockTillNextFrame == FALSE)
	// b) blit the current frame to the framebuffer
	// c) check to see if the last frame has been displayed.
	//    if so, terminate and unload the current movie, unless _bPlayLooping == TRUE.

	if( !_bModuleInstalled )
		return TRUE;

	BOOL bUnloadMovie = FALSE;
	if( _hBink ) {


		//if we are here, we are ready to decompress the next frame...
		if( _bNextFrameReady && !_bMoviePaused )
			_nSkippedFrames += BinkDoFrame( _hBink );

		//now transfer the current frame image to the frame buffer
//		if( !_BltGCFrame() )
//			return FALSE;
		if( !_BltGCTextureAndRender() ) {
			fmovie2_Unload();
			return FALSE;
		}
			

		//check to see if we have played our last frame
		//if so, set the unload flag

		if( _bNextFrameReady && !_bMoviePaused ) {
			if ( !(_uPlayFlags & FMOVIE2_PLAYFLAGS_LOOPING ) && ( _hBink->FrameNum == _hBink->Frames ) ) 
				bUnloadMovie = TRUE;
			else
				BinkNextFrame( _hBink );
		}
		BOOL bWaiting = FALSE;
		while( ( bWaiting = BinkWait( _hBink ) ) && !_bMoviePaused && bBlockTillNextFrame ) { 
			fgcfile_HandleDVDErrors( TRUE, TRUE ); 
		};

		_bNextFrameReady = !bWaiting;
	}

	//last thing to do is check to see if we need to unload the movie
	if( bUnloadMovie )
		fmovie2_Unload();

	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// PRIVATE IMPLEMENTATION FUNCTIONS

void* RADLINK _MovieAlloc ( u32 uNumBytes ) {
//	void* pMemory = fres_Alloc( uNumBytes );
	void* pMemory = FDS_StreamMgr.AllocFromCache( uNumBytes );
	DEVPRINTF( "[ FMOVIE2 ] _MovieAlloc -- Address %X, %d bytes\n", pMemory, uNumBytes );
	return pMemory; 
}


void RADLINK _MovieFree ( void* pMemToFree ) {
//	fmem_Free( pMemToFree );
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

	DVDDir        dir;
	DVDDirEntry   dirent;
	*puNumTableEntries = 0;
	char szSearchBasePath[ _MAX_PATH ];
	
	//first, copy the base path into the search base path local variable
	fclib_strcpy( szSearchBasePath, pszBasePath );
	//next, check to see if it has a trailing slash (for search purposes)
	u32 uSearchBasePathLength = fclib_strlen( szSearchBasePath );
	if( szSearchBasePath[ uSearchBasePathLength - 1 ] != '/' )
		fclib_strcat( szSearchBasePath, "/" );

	//now, append the wildcard character to the search string as well
//	fclib_strcat( szSearchBasePath, "*" );

	//now, open the DVD Directory
	if ( FALSE == DVDOpenDir( szSearchBasePath, &dir ) ) {
		DEVPRINTF( "[ FMOVIE2 ] _SearchDirectories -- Could not open DVD Directory Path %s!", szSearchBasePath );
		return FALSE;
	}
	
	//now, loop through each entry in the directory searching for directories and bink files
	while ( DVDReadDir(&dir, &dirent) ) {
		if ( bScanSubDirs && dirent.isDir ) {
			//recursively call this routine and search this directory
			char szSubDirFullPath[ _MAX_PATH ];
			u32 uSubDirNumTableEntries = 0;

			//construct the full path for this subdirectory
			fclib_strcpy( szSubDirFullPath, szSearchBasePath );
			//now append the subdirectory name to the end of the current path
			fclib_strcat( szSubDirFullPath, dirent.name );

			_MovieTableEntry_t *apSubDirTableHead = NULL;
			if( apTableHead )
				apSubDirTableHead = &apTableHead[ *puNumTableEntries ];
			_SearchDirectories( szSubDirFullPath, bScanSubDirs, apSubDirTableHead, &uSubDirNumTableEntries );
			*puNumTableEntries += uSubDirNumTableEntries;
		}
		else {
			//this is a regular filename, see if it's a bink file 
			char *pcFilename;
			BOOL bBinkFile = FALSE;
			for ( pcFilename = dirent.name; *pcFilename; pcFilename++ ) {
				if ( pcFilename[0] == '.' &&
					 pcFilename[1] == 'b' &&
					 pcFilename[2] == 'i' &&
					 pcFilename[3] == 'k' &&
					 pcFilename[4] == 0 ) {
					bBinkFile = TRUE;
					break;
				}
			}
			if( bBinkFile ) {
				if( apTableHead ) {
					//memory needs to be allocated to store the filename in...
					u32 uFilenameLengthPlusNull = fclib_strlen( dirent.name ) + 1;
					char* pszFileName = ( char* ) fres_Alloc( uFilenameLengthPlusNull );
					if( !pszFileName ) { //error allocating memory...
						DEVPRINTF( "[ FMOVIE2 ] _SearchDirectories -- Not enough memory to create Filename string!" );
						DVDCloseDir( &dir );
						return FALSE;
					}
					fclib_strcpy ( pszFileName, dirent.name );
					apTableHead[ *puNumTableEntries ].pszName = pszFileName;

					//now, construct the BASE PATH.  Put a null character in that
					//final trailing backslash for the next bit of work
					u32 uBasePathLengthPlusNull = fclib_strlen( szSearchBasePath );
					char* pszBasePath = ( char* ) fres_Alloc( uBasePathLengthPlusNull );
					if( !pszBasePath ) { //error allocating memory...
						DEVPRINTF( "[ FMOVIE2 ] _SearchDirectories -- Not enough memory to create BasePath string!" );
						DVDCloseDir( &dir );
						return FALSE;
					}
					fclib_strcpy ( pszBasePath, szSearchBasePath );
					apTableHead[ *puNumTableEntries ].pszPath = pszBasePath;
				}
                //lastly, increment the number of table entries...
				(*puNumTableEntries)++;
			}
		}
	}
	DVDCloseDir( &dir );	
	return TRUE;
}



BOOL _BltGCFrame( void ) {

	//This routine will copy the frame data to the GameCube back buffer


	//now, figure out the center position of the movie...
//	u32 uX = ( BackBufferSurfaceDesc.Width - _hBink->Width ) / 2;
//	u32 uY = ( BackBufferSurfaceDesc.Height - _hBink->Height ) / 2;

//RAFHACK -- take out hardcoded defines for screen size!!!
	void *pBufferToUse = FGCVid_pCurrentBuffer;
//	if( pBufferToUse == FGCVid_pCurrentBuffer )
//		pBufferToUse = FGCVid_pFrameBuffer2;
		
	BinkCopyToBuffer( _hBink,
    				  pBufferToUse,
    				  (S32)(512 * 2),
                      _hBink->Height,
                      0,0,
                      BINKSURFACEYUY2 |
                      BINKCOPYALL );

    DCStoreRange( pBufferToUse, 512*2*_hBink->Height );
//    DCStoreRange( pBufferToUse, FGCVid_nFrameBufferSize );

	return TRUE;
}



BOOL _BltGCTextureAndRender( void ) {

	//This routine will get the fullscreen texture surface available from
	//the fsh interfaces and blast the next frame onto that texture.
	//Then that texture will be rasterized 
	FDrawVtx_t Verts[6];
	CFColorRGBA Color;
	FDrawVtx_t *pVert = &Verts[0];
	CFTexInst *pFullscreenTex = fsh_GetFullScrTexture();
	FTexDef_t *pTexDef = pFullscreenTex->GetTexDef();

	u32 uX = ( pTexDef->TexInfo.nTexelsAcross - _hBink->Width ) / 2;
	u32 uY = ( pTexDef->TexInfo.nTexelsDown - _hBink->Height ) / 2;
	
	//first, update the fullscreen texture with the latest movie frame
	u16 *pTextureData = fsh_GetFullScrData();
	if ( !pTextureData )
	{
		return FALSE;
	}

	if( _hBink->FrameNum == 1 ) {
		//we want to clear the backbuffer to all black the first frame
		fang_MemSet(pTextureData, 0, pTexDef->TexInfo.nTexelsAcross * pTexDef->TexInfo.nTexelsDown * 2); 
	}		
	
    BinkCopyToBuffer( _hBink,
                      pTextureData,
                      (S32)pTexDef->TexInfo.nTexelsAcross * 2, //16 bit format
                      _hBink->Height,
                      uX, uY,
                      BINKSURFACE565 | BINKCOPYALL );

	DCStoreRange(pTextureData, pTexDef->TexInfo.nTexelsAcross * pTexDef->TexInfo.nTexelsDown * 2);
	fsh_ReleaseFullScrData();
	
	// Now, Prep the fdraw quad for rendering
	FViewport_t *pPreviousVP = fviewport_GetActive();
	u32 nWidth = FViewport_pDefaultOrtho->nWidth;
	u32 nHeight = FViewport_pDefaultOrtho->nHeight;
	Color.Set( 1.f, 1.f, 1.f, 1.f );

	pVert->Pos_MS.Set( nWidth, 0.f, 1.f );
	pVert->ColorRGBA = Color;
	pVert->ST.Set( 1.f, 0.f );
	pVert++;
	pVert->Pos_MS.Set( nWidth, nHeight, 1.f );
	pVert->ColorRGBA = Color;
	pVert->ST.Set( 1.f, 1.f );
	pVert++;
	pVert->Pos_MS.Set( 0.f, 0.f, 1.f );
	pVert->ColorRGBA = Color;
	pVert->ST.Set( 0.f, 0.f );
	pVert++;
	pVert->Pos_MS.Set( 0.f, nHeight, 1.f );
	pVert->ColorRGBA = Color;
	pVert->ST.Set( 0.f, 1.f );
	pVert++;
	pVert->Pos_MS.Set( 0.f, 0.f, 1.f );
	pVert->ColorRGBA = Color;
	pVert->ST.Set( 0.f, 0.f );
	pVert++;
	pVert->Pos_MS.Set( nWidth, nHeight, 1.f );
	pVert->ColorRGBA = Color;
	pVert->ST.Set( 1.f, 1.f );
	pVert++;
	
	// Set the performance monitoring viewport and fdraw renderer
	CFXfm CamXfm;
	fviewport_SetActive( FViewport_pDefaultOrtho );
	frenderer_Push( FRENDERER_DRAW, NULL );
	CamXfm.Identity();
	CamXfm.InitStackWithView();
		
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECALTEX_AI );
	fdraw_SetTexture( pFullscreenTex );

	//submit the quad for rendering!
	fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, Verts, 6 );

	// Restore original state
	frenderer_Pop();
	fviewport_SetActive( pPreviousVP );
	
	return TRUE;
}