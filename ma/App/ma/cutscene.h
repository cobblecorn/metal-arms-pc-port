//////////////////////////////////////////////////////////////////////////////////////
// cutscene.h - in-game cutscene movie player 
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
// 01/15/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CUTSCENE_H_
#define _CUTSCENE_H_ 1

#include "fang.h"

#define CUTSCENE_INVALID_HANDLE -1

typedef u32 cutscene_Handle_t;


extern BOOL cutscene_InitSystem( void );
extern void cutscene_UninitSystem( void );

//gets a handle to a cutscene
//  pszMovieFilename:			The name of the movie
//	bPrependPlatformPrefix:     If TRUE, prepend "XB_" or "GC_" to the filename passed
//	                            in.  This way, platform independent filenames can be 
//                              passed into this function.
extern cutscene_Handle_t cutscene_AcquireHandle( cchar* pszMovieFilename, 
												 BOOL bPrependPlatformPrefix = TRUE );

//This function is meant to be called during realtime game playing operations.
//It halts the game for the duration of the movie.
//PARAMETERS:
//	hCutscene :                 Handle to the cutscene to be played
//	fNormalizedVolume:			Volume level from 0.0 to 1.0f to play the movie at
extern BOOL cutscene_Start( cutscene_Handle_t hCutscene, 
						    f32 fNormalizedVolume = 1.0f );

//this will terminate a cutscene movie if called.
extern void cutscene_End( void );

#endif

