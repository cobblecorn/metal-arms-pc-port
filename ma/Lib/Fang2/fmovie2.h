//////////////////////////////////////////////////////////////////////////////////////
// fmovie2.h - Fang movie player v2.
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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef _FMOVIE2_H_
#define _FMOVIE2_H_ 1

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fang.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//DEFINES HERE

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef enum
{
	FMOVIE2_NO_ERROR	=	0,
	FMOVIE2_ERROR		=	-1

} FMovie2_Error_e;

typedef enum {
	FMOVIE2_STATUS_UNLOADED,
	FMOVIE2_STATUS_PLAYING,
	FMOVIE2_STATUS_PAUSED
} FMovie2Status_e;

enum { //bitfields describing various play options
	FMOVIE2_PLAYFLAGS_LOOPING					= 0x01,
	FMOVIE2_PLAYFLAGS_LOADENTIREMOVIEINTOMEMORY	= 0x02,

	FMOVIE2_PLAYFLAGS_NONE						= 0x00
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//// Module.
//
extern BOOL fmovie2_ModuleStartup( void );
extern void fmovie2_ModuleShutdown( void );
extern FMovie2_Error_e fmovie2_Install( void ); 
extern void fmovie2_Uninstall( void );
extern BOOL fmovie2_IsInstalled( void );

//
////

//// Interface.
//
extern void fmovie2_Play( cchar *pszFileName, 
						  f32 fVolume = 1.0f,
						  cu32 uPlayFlags = FMOVIE2_PLAYFLAGS_NONE );
extern void fmovie2_Pause( BOOL bPause );
extern void fmovie2_Unload( void );
extern FMovie2Status_e fmovie2_GetStatus( void );
extern BOOL fmovie2_MovieExists( cchar *pszFileName );

//
////

//// Misc maintenance.
//

//CALL THIS FUNCTION TO DRAW THE MOVIE TO THE BACKBUFFER.
//  If bBlockTillNextFrame == TRUE, then this function will block until it's time to display the next frame.
//  If bBlockTillNextFrame == FALSE, then this function will return immediately after drawing the frame, 
//     however, it is the responsibility of the app to call fmovie2_TimeToUpdate with a very high degree
//     of frequency.
extern BOOL fmovie2_Draw( BOOL bBlockTillNextFrame = TRUE ); //call this function to have the movie draw to the Destination buffer 

//Use this function when you don't want to block on draw calls.  This function MUST be called with a high degree of 
//frequency.
//This function returns TRUE when it's time to call fmovie2_Draw, and false otherwise.
extern BOOL fmovie2_TimeToDraw( void );
//
////


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif // _FMOVIE2_H_

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
