//////////////////////////////////////////////////////////////////////////////////////
// fvid.h - Fang video system module.
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
// 09/14/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FVID_H_
#define _FVID_H_ 1

#include "fang.h"

#if FANG_PLATFORM_WIN
	#include <windows.h>
#endif



#define FVID_DEVNAME_LEN		63
#define FVID_WINTITLE_LEN		63


// When bSuspend is TRUE, the OS is requesting permission to place the computer into a suspended state.
// In this case, if the app wishes to grant the request, it must take appropriate action (save data, etc.)
// and then return TRUE. To deny, return FALSE. If bSuspend is FALSE, the computer is coming out of
// suspend mode and the app should restore its data. In this case, the return value is ignored.
typedef BOOL FVidSuspendFcn_t( BOOL bSuspend );

typedef void FVidDrawOverlayFcn_t( void );


typedef enum {
	FVID_RENDERER_HARDWARE,				// Use hardware renderer
	FVID_RENDERER_SOFTWARE,				// Use software renderer
	FVID_RENDERER_EMULATION,			// Use emulation renderer

	FVID_RENDERER_COUNT
} FVidRenderer_t;


enum {
	FVID_DEVFLAG_HW_TNL			= 0x0001,	// Hardware TnL (always set for X-Box and PS2)
	FVID_DEVFLAG_NONE			= 0x0000
};

enum {
	FVID_MODEFLAG_WINDOWED		= 0x01,		// Mode runs in a window on the desktop (never set for consoles)
	FVID_MODEFLAG_INTERLACED	= 0x02,		// Mode is interlaced (never set for Windows)
	FVID_MODEFLAG_PAL_60HZ		= 0x04,		// Mode is 60 Hz PAL as opposed to usual 50 Hz (never set for Windows)
	FVID_MODEFLAG_NONE			= 0x00
};


typedef struct {						// Device Info:
	char szName[FVID_DEVNAME_LEN+1];	// ASCIIZ device name
	u16 nFlags;							// See FVID_DEVFLAG_* for info
	u8 nOrdinal;						// Normally 0, but incremented if the system has more than one device with
										//   the same szName.
	u8 nRenderer;						// See FVidRenderer_t for info
} FVidDev_t;


typedef struct {						// Mode Info:
	u8 nFlags;							// See FVID_MODEFLAG_* for info
	u8 nColorBits;						// Bit depth of the color buffer
	u8 nDepthBits;						// Bit depth of the depth buffer
	u8 nStencilBits;					// Bit depth of the stencil buffer
	u16 nPixelsAcross;					// Number of pixels across
	u16 nPixelsDown;					// Number of pixels down
} FVidMode_t;


typedef struct {
	FVidDev_t VidDev;					// Requested device
	FVidMode_t VidMode;					// Requested mode

	u32 nSwapInterval;					// Describes when to perform the swap (0=instantly, 1=next vertical blank, etc.)
	f32 fUnitFSAA;						// Amount of FSAA: 0.0=none, 1.0=max

#if FANG_PLATFORM_WIN
	HINSTANCE hInstance;				// Application windows's hInstance (for X-Box and Win only)

	char szWindowTitle[FVID_WINTITLE_LEN+1];// ASCIIZ string to be displayed on windowed-mode titlebar
											//   (set first char to NULL for no title)

	HWND hWnd;							// Optional Window handle. If the game has already created a rendering
										//   window, place that window's HWND into this field. Otherwise,
										//   place NULL into this field and Fang will create a window.

	DWORD nIconIDI;						// Optional icon for PC windowed mode (NULL: use standard app icon)

	BOOL bAllowPowerSuspend;			// TRUE=allow Windows to put the computer into a suspended state
	FVidSuspendFcn_t *pFcnSuspend;		// Pointer to app's suspend callback function (NULL=none)
#endif
} FVidWin_t;



extern BOOL FVid_bOnline;				// TRUE: Video system is online

// These are valid only when FVid_bOnline is TRUE:
extern FVidDev_t FVid_Dev;				// Current device
extern FVidMode_t FVid_Mode;			// Current video mode
extern FVidWin_t FVid_Win;				// Current window info

extern BOOL FVid_bBegin;				// TRUE: Inside a fvid_Begin()/fvid_End() pair

extern f32 FVid_fBrightness;			// 1.f is default brightness.  There is clamping that varies on a platform basis
extern u32 FVid_nSwapCounter;
extern u32 FVid_nFrameCounter;			// Increments each time fvid_Begin() is called
extern BOOL FVid_bPrintHiFreqErrMsg;	// Set to TRUE for one frame every few seconds. Can be used to limit the number of error messages printed.



//---------------------------------------------------------------------------------------------------------------------
// fvid_ModuleStartup()
// fvid_ModuleShutdown()
//
// Starts up and shuts down this module, respectively.
// Called by the Fang initialization system.
// Returns TRUE if successful, or FALSE if the module could not successfully initialized.
extern BOOL fvid_ModuleStartup( void );
extern void fvid_ModuleShutdown( void );



//---------------------------------------------------------------------------------------------------------------------
// fvid_Enumerate()
// fvid_EnumerateReset()
// fvid_HasEnumerated()
// fvid_GetDeviceCount()
// fvid_GetDeviceInfo()
// fvid_GetModeCount()
// fvid_GetModeInfo()
//
// These functions are used to enumerate the graphics system devices and modes.
//
// Call fvid_Enumerate() to perform the enumeration. The function may take a few seconds to
// return. Once fvid_Enumerate() has returned, the app is free to use the Get functions to
// retrieve information about the available devices and modes. Each time fvid_Enumerate()
// is called, the lengthy enumeration might be performed.
//
// The enumeration data collected by fvid_Enumerate() is kept in memory until the module
// is shutdown, or if fvid_EnumerateReset() is called. If fvid_EnumerateReset() is called,
// the enumeration memory is released. The Reset function is useful when the game knows it
// won't need to enumerate any further and can, therefore, free memory consumed by enumeration
// tables.
//
// The nRenderer parameter passed to fvid_Enumerate() informs PC-based systems which renderer
// to use. For X-Box and PlayStation2, this parameter is ignored.
//
// fvid_HasEnumerated() returns TRUE if the enumeration data is still in memory, and fills
// *pnRenderer (if not NULL) with the last enumerated renderer.
//
// fvid_GetDeviceCount() returns the number of devices (adapters) on the system. 0=none.
// fvid_GetDeviceInfo() returns a pointer to information about the specified device.
// fvid_GetModeCount() returns the number of modes for the specified device. 0=none
// fvid_GetModeInfo() returns a pointer to information about the specified mode.
//
// Modes are sorted in order of bWindowed, nColorBits, nDepthBits, nStencilBits, nPixelsAcross,
// and nPixelsDown. Devices are not sorted.
extern void fvid_Enumerate( FVidRenderer_t nRenderer );
extern void fvid_EnumerateReset( void );
extern BOOL fvid_HasEnumerated( FVidRenderer_t *pnRenderer );
extern u32 fvid_GetDeviceCount( void );
extern const FVidDev_t *fvid_GetDeviceInfo( u32 nDevIndex );
extern u32 fvid_GetModeCount( u32 nDevIndex );
extern const FVidMode_t *fvid_GetModeInfo( u32 nDevIndex, u32 nModeIndex );



//---------------------------------------------------------------------------------------------------------------------
// fvid_CreateWindow()
// fvid_DestroyWindow()
// fvid_IsMinimized()
// fvid_HasUserClosedWindow()
//
// fvid_CreateWindow() and fvid_DestroyWindow() creates and destroys the video window,
// respectively. The window is created on the specified device, and in the specified
// mode, as described by the pWinInfo parameter. If enumeration has not yet been
// performed, fvid_CreateWindow() calls fvid_Enumerate(). When fvid_CreateWindow()
// returns successfully, fvid_HasEnumerated() will always return TRUE. If unsuccessful,
// fvid_HasEnumerated() may return either TRUE or FALSE.
//
// fvid_CreateWindow() will return TRUE if the window has been created, and FALSE if
// the window could not be created. Once the window has be created, Fang's graphics
// system may be used. Note that although enumeration lists modes that are supported
// by the hardware, fvid_CreateWindow() may still fail do to any number of problems
// including but not limited to lack of video memory. Check the return value of
// fvid_CreateWindow().
//
// Use fvid_DestroyWindow() to destroy the window. When this function returns, Fang's
// graphic system cannot be used.
//
// pWinInfo fields:
//
// VidDev and VidMode indicate the desired device and mode for the window to be created.
//
// nSwapInterval indicates the timing involved in swapping the front and back buffers.
// If 0, fvid_Swap() will swap the buffers immediately. If 1, the fvid_Swap() function
// will return immediately, but the swap will take place during the next vertical blank.
// If 2, the swap will occur on the second vertical blank. Valid values are 0, 1, and 2.
// Not all systems support values other than 0. Check FVid_Win::FVid_nSwapInterval to
// determine the swap interval actually used.
//
// fUnitFSAA indicates the degree of full scene anti-aliasing that should be imployed.
// A value of 0.0f indicates that no FSAA should be used. A value of 1.0f indicates that
// maximum FSAA should be used. The larger the number, the lower the performace of the
// graphics system. Not all systems support FSAA. Check FVid_Win::FVid_fUnitFSAA to
// determine the FSAA value actually used.
//
// On PC and X-Box, hInstance indicates the application's HINSTANCE.
//
// On PC, nIconIDI is an optional icon IDI resource that indicates the icon to use in
// windowed mode. If NULL, a default icon is used.
//
// On PC, szWindowTitle holds a string that is displayed in the title bar of a windowed
// window. For no title, set the first character in szWindowTitle to NULL.
//
// On PC, bAllowPowerSuspend and pFcnSuspend describe how the game wishes to handle
// power-suspension by Windows. If bAllowPowerSuspend is FALSE, pFcnSuspend is ignored
// and Windows will not be permitted to enter a power-suspension state. If bAllowPowerSuspend
// is TRUE, then pFcnSuspend is checked. If pFcnSuspend is NULL, Windows will be permitted
// to enter a suspended state. If pFcnSuspend is not NULL, it must point to a function
// in the game. This function accepts a parameter: TRUE=Windows entering suspend mode, FALSE=
// Windows exiting suspend mode. When FALSE, the function's return value is ignored. But,
// when the parameter is TRUE, the function may return TRUE to allow Windows to enter
// the suspend mode, or FALSE to deny it. The function is also used by the application
// to save and restore its state.
//
// On a PC, it is possible to create a windowed graphics context. Once created, the size
// may not be changed via the GUI. However, the window may be moved, minimized and restored.
// fvid_IsMinimized() can be used by the app to determine whether the window is currently
// minimized. For consoles, fvid_IsMinimized() always returns FALSE.
//
// Also, if the user manually closes a windowed window (by ALT-F4 or clicking on the X),
// the Fang video system will set a flag. This flag can be checked by calling
// fvid_HasUserClosedWindow(). The funciton returns TRUE if the user has requested that
// the window be closed. This function is usally called by the game's main loop. It always
// returns FALSE for PS2 and X-Box.
extern BOOL fvid_CreateWindow( const FVidWin_t *pWinInfo );
extern BOOL fvid_ResetWindow( const FVidWin_t *pWinInfo );
extern void fvid_DestroyWindow( void );
extern BOOL fvid_IsMinimized( void );
extern BOOL fvid_HasUserClosedWindow( void );

extern FVidDrawOverlayFcn_t *fvid_GetDrawOverlayFcn( void );
extern void fvid_SetDrawOverlayFcn( FVidDrawOverlayFcn_t *pFcnDrawOverlay );


//---------------------------------------------------------------------------------------------------------------------
// fvid_Begin()
// fvid_End()
//
// The app must use the Begin and End functions to tell Fang when it plans on drawing.
// fvid_Begin() should be placed as late as possible in the game frame, but before
// any drawing takes place. fvid_End() should be placed as soon as possible, and after
// all drawing for the frame has occured. Only one Begin/End pair is permitted per
// frame.
//
// fvid_Begin() also increments FVid_nFrameCounter by 1.
//
// fvid_Begin() returns TRUE if the graphics window is in a state that can be seen.
// This is the normal return value. If FALSE is returned, the graphics window is either
// minimized, or obscured somehow, and rendering is not necessary. The return value can
// be used as a hint to the application as to whether to proceed with the execution of
// graphics code or to simply skip the graphics code and simply call fvid_End().
// Regardless of the return value, FVid_nFrameCounter is incremented by 1. On X-Box and
// PlayStation2, fvid_Begin() always returns TRUE.
extern BOOL fvid_Begin( void );
extern void fvid_End( void );



//---------------------------------------------------------------------------------------------------------------------
// fvid_Swap()
// fvid_Flush()
//
// fvid_Swap() swaps the color buffers, presenting the contents of the back buffer
// to the screen as set up by the nSwapInterval parameter given to fvid_StartupVideoSystem().
// Only one fvid_Swap() is permitted per frame. fvid_Swap() should occur outside the
// fvid_Begin()/fvid_End() pair, but as far from fvid_Begin() as possible.
//
// fvid_Flush() blocks until all background rendering has finished. This function is necessary
// when the game wishes to modify any geometry or dynamically-generated texture data.
// For best performance, place a call to fvid_Flush() as close to and before fvid_Begin().
// When fvid_Flush() returns, geometry and dynamic textures may be modified.
//
// fvid_Swap() returns TRUE if successful, or FALSE if a catastrophic
// error has occurred. An error can occur only on the Windows implementation. If FALSE
// is returned, the video system has automatically called fvid_DestroyWindow() and
// has destroyed all graphics objects. In response, the application can either attempt
// to re-create the window, or can report an error to the user.
extern BOOL fvid_Swap( void );
extern void fvid_Flush( void );



//---------------------------------------------------------------------------------------------------------------------
// fvid_ResetFrameCounter()
// fvid_IncrementFrameCounter()
//
// These are utility functions used to manually manage FVid_nFrameCounter. Note that
// fvid_Begin() calls fvid_IncrementFrameCounter().
extern void fvid_ResetFrameCounter( void );
extern void fvid_IncrementFrameCounter( void );

extern u32 fvid_GetScreenGrabBufferSizeInBytes( void );
extern BOOL fvid_ScreenGrab( void *pDestBuffer, u32 nBufferBytes );
extern void fvid_SetGamma( f32 fUnitGamma );



#endif

