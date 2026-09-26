//////////////////////////////////////////////////////////////////////////////////////
// fGCfile.h - Fang file module.
//
// Author: John Lafleur
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
// 06/11/02	Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "ffile.h"



//
//
//
class CFPlatformFileCookie : public CFFileCookie
{
	public:
		s32			m_nEntryNum;	// GC Directory entry number
		s32			m_nSeekPosition;// Current pending seek
		u32			m_nPosition;	// Current position in the file
		s32			m_nSize;		// Size of the file
		DVDFileInfo	m_FileInfo;		// GC FileInfo
		void		*m_pLastDestBuffer;
		FFileAsyncReadCallback_t *m_pCallback;
		void		*m_pUser;		// User data returned back to m_pCallback
};



extern BOOL fgcfile_ModuleStartup( void );
extern BOOL fgcfile_ModuleShutdown( void );

// These functions always access files within the standard OS directory hierarchy:
extern FFileHandle ffile_OSDir_Open( cchar *pszFName, FFileOpenType_e nOpenMode, BOOL bAsyncAndAligned = FALSE );
extern s32 ffile_OSDir_Close( CFPlatformFileCookie *pCookie );
extern s32 ffile_OSDir_GetFileSize( CFPlatformFileCookie *pCookie );
extern s32 ffile_OSDir_Seek( CFPlatformFileCookie *pCookie, s32 nFileOffset, FFileSeekType_e nSeekMode );
extern s32 ffile_OSDir_Tell( CFPlatformFileCookie *pCookie );
extern s32 ffile_OSDir_Read( CFPlatformFileCookie *pCookie, u32 uReadAmt, void *pDestData, FFileAsyncReadCallback_t *pCallback = NULL, void *pUser = NULL );
extern s32 ffile_OSDir_Write( CFPlatformFileCookie *pCookie, u32 uWriteAmt, void *pSrcData );
extern s32 ffile_OSDir_EOF( CFPlatformFileCookie *pCookie );
extern s32 ffile_OSDir_Scanf( CFPlatformFileCookie *pCookie, const char *szFmt, ... );

// Input Params:
// 	bDetectError: Will call the OS level API to detect errors...
//  bBlockWhileError: Will actually block while the error condition is true.	
// A Return Value of FALSE means there were no Errors
// A Return Value of TRUE means there was an error and it was addressed.
// This routine can optionally detect drive errors (for synchronous cases)
extern BOOL fgcfile_HandleDVDErrors( BOOL bDetectError, BOOL bBlockWhileError ); 
