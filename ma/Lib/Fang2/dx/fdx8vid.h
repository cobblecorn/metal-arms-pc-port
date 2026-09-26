//////////////////////////////////////////////////////////////////////////////////////
// fdx8vid.h - Fang video system module (DX8 version).
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 10/02/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FDX8VID_H_
#define _FDX8VID_H_ 1

#include "fang.h"
#include "fdx8.h"
#include "fvid.h"


typedef enum {
	FDX8VID_EVENT_WINDOW_CREATED,
	FDX8VID_EVENT_WINDOW_DESTROYED,
	FDX8VID_EVENT_PRE_RESET,
	FDX8VID_EVENT_POST_RESET,

	FDX8VID_EVENT_COUNT,
} FDX8VidEvent_e;


typedef void FDX8VidMouseCallback_t( u32 nX, u32 nY, s32 nDeltaWheel, BOOL bLeftButton, BOOL bRightButton, void *pParm );


// Modules may register a window creation/destruction callback function with the
// video system. When the window is created, the video system calls, in order of
// registration, each registered callback function passing bCreated=TRUE. The
// callback function must return TRUE if it wishes to permit the window to be
// created, or FALSE if the window should not be created. When the window is
// destroyed, the video system calls, in revers-order of registration, each
// registered callback function passing bCreated=FALSE. In this case the value
// returned by the callback function is ignored.
typedef BOOL FDX8VidWindowCallback_t( FDX8VidEvent_e nEvent );


extern D3DMULTISAMPLE_TYPE FDX8Vid_nBackBufferMultisampleType;
extern BOOL fdx8vid_bResetting;	// TRUE: Only ->Release() may be used


extern void fdx8vid_RegisterWindowCallbackFunction( FDX8VidWindowCallback_t *pFcnWindowCallback );
extern void fdx8vid_UnregisterWindowCallbackFunction( FDX8VidWindowCallback_t *pFcnWindowCallback );
extern BOOL fdx8vid_IsWindowCallbackFunctionRegistered( FDX8VidWindowCallback_t *pFcnWindowCallback );

extern void fdx8vid_RegisterMouseCallback( FDX8VidMouseCallback_t *pMouseCallback, void *pParam );
extern FDX8VidMouseCallback_t *fdx8vid_GetMouseCallback( void );
extern void *fdx8vid_GetMouseCallbackParm( void );

extern BOOL fdx8vid_CheckDeviceFormat( DWORD nUsageFlags, D3DRESOURCETYPE nResourceType, D3DFORMAT nFormatToCheck );
extern BOOL fdx8vid_CheckDepthStencilMatch( D3DFORMAT nColorFormat, D3DFORMAT nDepthStencilFormat );

extern HWND fdx8vid_GetWindowHandle( void );
extern void fdx8vid_MinimizeWindow( void );
extern void fdx8vid_RestoreWindow( void );

extern BOOL fdx8vid_IsDeviceLost( void );
extern BOOL fdx8vid_HaveFocus( void );

extern void fdx8vid_EnableCursor( BOOL bEnable );
extern BOOL fdx8vid_SetCursor( HCURSOR hCursor );
extern void fdx8vid_SetCursorPosition( u32 nScreenspaceX, u32 nScreenspaceY );
extern void fdx8vid_GetCursorPosition( u32 *pnScreenspaceX, u32 *pnScreenspaceY );


#endif

