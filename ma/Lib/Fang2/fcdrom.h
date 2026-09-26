//////////////////////////////////////////////////////////////////////////////////////
// fcdrom.h - Fang cdrom.
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
// 06/11/01 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef _FCDROM_H_
#define _FCDROM_H_ 1

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/*
Notes on using FCdrom:

- First call should be fcdrom_ModuleStartup().
- Last call should be fcdrom_ModuleShutdown().
- Nothing can be called until fcdrom_ModuleStartup() is called or after fcdrom_ModuleShutdown() is called.

- Streaming and reading from files are mutually exclusive.
- All file names on the CD must be globally unique to the CD.

- Limitation imposed by the ISO 9660 CD standard:

   * No files can be stored in a directory more than 8 levels deep. The root directory counts as level 1.
   * Directory names can't contain extensions, and can't be more than 8 characters.
   * File names don't require an extension, and can't be more than 8 characters for the name, and 3 characters for the extension.
   * Valid characters for file and directory names are limited to: "0" through "9", "A" to "Z", and "_" (underscore).

- The rest should be pretty self-explanatory, otherwise please consult the author and current maintainer:

Albert Yale
ayale@swinginape.com
949-716-8209 x 206
*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fang.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define FCDROM_FILES_ON_DISK_MAX				256
#define FCDROM_FILES_OPENED_MAX					50
#define FCDROM_FILENAME_LENGTH_MAX				( 14 + 1 )	// Max char length + NULL char. filename.ext;1
#define FCDROM_FILENAME_AND_PATH_LENGTH_MAX		( 85 + 1 )	// Max char length + NULL char. cdrom0:\dirname2\dirname3\dirname4\dirname5\dirname6\dirname7\dirname8\filename.ext;1

#define FCDROM_MAX_STREAMS						1

#define FCDROM_STREAMS_MIN_VOLUME				0
#define FCDROM_STREAMS_MAX_VOLUME				0xff

#define FCDROM_STREAMS_MAX_PAN_LEFT				0
#define FCDROM_STREAMS_PAN_MIDDLE				0x7f
#define FCDROM_STREAMS_MAX_PAN_RIGHT			0xfe

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef u32 FCdromHandle_t;

typedef enum
{
	FCDROM_SEEK_SET = 0,
	FCDROM_SEEK_CUR,
	FCDROM_SEEK_END,

	FCDROM_SEEK_COUNT

} FCdromSeekOrigins_e;

typedef enum
{
	FCDROM_NO_ERROR		=	 0,
	FCDROM_ERROR		=	-1

} FCdromError_e;

typedef enum
{
	FCDROM_STREAM_STOPPED = 0,
	FCDROM_STREAM_STARTING,
	FCDROM_STREAM_PLAYING,
	FCDROM_STREAM_PAUSED

} FCdromStreamStatus_e;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern BOOL fcdrom_ModuleStartup( void );
extern void fcdrom_ModuleShutdown( void );
extern s32 fcdrom_Seek( const FCdromHandle_t hID, const u32 uOffset, const FCdromSeekOrigins_e oOrigin );
extern s32 fcdrom_Read( const FCdromHandle_t hID, const u32 uLength, void *pDestination );
extern FCdromError_e fcdrom_Size( const FCdromHandle_t hID, u32 *puLength );
extern FCdromError_e fcdrom_Open( const char *pszName, FCdromHandle_t *phID );
extern FCdromError_e fcdrom_Close( const FCdromHandle_t hID );
extern FCdromError_e fcdrom_NameFromHandle( const FCdromHandle_t hID, char *pszName );
extern FCdromError_e fcdrom_CurrentPosition( const FCdromHandle_t hID, u32 *puPosition );
extern FCdromError_e fcdrom_StreamPlay( const char *pszName, const BOOL bBlock, const u8 uVolume, const u8 uPan, const BOOL bLooping );
extern FCdromError_e fcdrom_StreamStatus( FCdromStreamStatus_e *oStatus );
extern FCdromError_e fcdrom_StreamStop( void );
extern FCdromError_e fcdrom_StreamGetName( char *pszName );
extern FCdromError_e fcdrom_StreamGetSampleRate( u32 *pSampleRate );
extern s32 fcdrom_StreamGetTotalTime( void ); // In seconds.
extern s32 fcdrom_StreamGetElapsedTime( void ); // In seconds.
extern FCdromError_e fcdrom_StreamPause( const BOOL bPause );
extern s32 fcdrom_StreamIsPaused( void );
extern FCdromError_e fcdrom_StreamSetVolume( const u8 uVolume );
extern s32 fcdrom_StreamGetVolume( void );
extern FCdromError_e fcdrom_StreamSetPan( const u8 uPan );
extern s32 fcdrom_StreamGetPan( void );

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
