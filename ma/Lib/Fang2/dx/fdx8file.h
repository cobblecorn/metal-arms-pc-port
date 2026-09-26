//////////////////////////////////////////////////////////////////////////////////////
// fdx8file.h - Fang file module.
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
#if FANG_PLATFORM_XB
	#include "xtl.h"
#else
	#include <windows.h>
#endif



//
//
//
class CFPlatformFileCookie : public CFFileCookie
{
	public:
		HANDLE	m_hFile;
		u32		m_nPosition;
		s32		m_nSeekPosition;

		// Async Read Info
		OVERLAPPED		m_Overlapped; 
		void			*m_pUser;
		u32				m_nAsyncReadAmount;
		volatile BOOL8	m_bAsyncReadReady;
		volatile BOOL8	m_bAsyncReadInProgress;
		u8				__PAD[2];
		void			*m_pLastDestBuffer;
		FFileAsyncReadCallback_t *m_pCallback;
};


extern BOOL fdx8file_ModuleStartup( void );
extern BOOL fdx8file_ModuleShutdown( void );


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

