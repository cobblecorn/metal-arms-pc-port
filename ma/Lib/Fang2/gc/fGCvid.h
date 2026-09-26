//////////////////////////////////////////////////////////////////////////////////////
// fGCvid.h - Fang video system module (GameCube version).
//
// Author: John Lafleur
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
// 02/18/02	Lafleur		Created from stubbed DX version
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FGCVID_H_
#define _FGCVID_H_ 1

#include "fang.h"
#include "fGC.h"
#include "fvid.h"


typedef enum 
{
	FGCVID_EVENT_WINDOW_CREATED,
	FGCVID_EVENT_WINDOW_DESTROYED,
	FGCVID_EVENT_PRE_RESET,
	FGCVID_EVENT_POST_RESET,

	FGCVID_EVENT_COUNT,
} FGCVidEvent_e;


extern void	*FGCVid_pFrameBuffer1;
extern void	*FGCVid_pFrameBuffer2;
extern void	*FGCVid_pCurrentBuffer;
extern u32  FGCVid_nFrameBufferSize;
extern void *FGCVid_pGraphicsFifo;
extern u32  FGCVid_nGraphicsFifoSize;


// Modules may register a window creation/destruction callback function with the
// video system. When the window is created, the video system calls, in order of
// registration, each registered callback function passing bCreated=TRUE. The
// callback function must return TRUE if it wishes to permit the window to be
// created, or FALSE if the window should not be created. When the window is
// destroyed, the video system calls, in revers-order of registration, each
// registered callback function passing bCreated=FALSE. In this case the value
// returned by the callback function is ignored.
typedef BOOL FGCVidWindowCallback_t( FGCVidEvent_e nEvent );


extern BOOL FGCvid_bResetting;	// TRUE: Only ->Release() may be used


extern void fgcvid_RegisterWindowCallbackFunction( FGCVidWindowCallback_t *pFcnWindowCallback );
extern void fgcvid_UnregisterWindowCallbackFunction( FGCVidWindowCallback_t *pFcnWindowCallback );
extern BOOL fgcvid_IsWindowCallbackFunctionRegistered( FGCVidWindowCallback_t *pFcnWindowCallback );

extern void fgcvid_EnableGPHangDiagnose( u32 nTimeoutFrames );
extern void fGCvid_Flush( BOOL bBlock );

#endif

