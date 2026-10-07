//////////////////////////////////////////////////////////////////////////////////////
// fdx8vid.cpp - Fang video system module (DX8 version).
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
// 09/18/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#if defined(MA_PC_INPUT)
#include "pc_input.h"
#endif

#include "fdx8.h"
#include "fdx8vid.h"
#include "fdx8tex.h"
#include "fdx8draw.h"
#include "ftext.h"
#include "frenderer.h"
#include "fcolor.h"
#include "fmovie2.h"
#include "fmesh.h"
#include "fclib.h"

#include "floop.h"
#include "fres.h"
#include "fperf.h"
#include "fsysinfo.h"

#include "sas_user.h"

#if FANG_PLATFORM_XB
	#include "D3d8perf.h"
#endif

#include <tchar.h>


//////////////////////////////////////////////////////////////////////////////////////
// Global Variables:
//////////////////////////////////////////////////////////////////////////////////////

BOOL FVid_bOnline;				// TRUE: Video system is online
f32  FVid_fBrightness;
BOOL FVid_bBegin;				// TRUE: Inside a fvid_Begin()/fvid_End() pair
u32  FVid_nFrameCounter;		// Increments each time fvid_Begin() is called
u32  FVid_nSwapCounter;
BOOL FVid_bPrintHiFreqErrMsg;	// Set to TRUE for one frame every few seconds. Can be used to limit the number of error messages printed.
BOOL fdx8vid_bResetting;		// TRUE: The device is being reset: only ->Release() can be used
#if FANG_WINGC
BOOL FDX8Vid_bPortBorderless = FALSE;	// a windowed mode drawn as a captionless window at the desktop origin
#endif

// These are valid only when FVid_bOnline is TRUE:
FVidDev_t  FVid_Dev;			// Current device
FVidMode_t FVid_Mode;			// Current video mode
FVidWin_t  FVid_Win;			// Current window info

D3DMULTISAMPLE_TYPE FDX8Vid_nBackBufferMultisampleType;


//////////////////////////////////////////////////////////////////////////////////////
// Local Defines:
//////////////////////////////////////////////////////////////////////////////////////

#define WM_MOUSEWHEEL							0x020A		// Not sure why this symbol isn't defined when I compile

#define _MAX_WINDOW_CREATION_CALLBACK_FCNS		20
#define _MAX_VIDMODES							500
#define _MAX_D3D_MODES_PER_ADAPTER				200
#define _MAX_D3D_FORMATS_PER_ADAPTER			20
#define _WINDOW_CLASS_NAME						_T("Fang D3D Window")

#define _OVERRIDE_SWAP_INTERVAL					(FALSE & (SAS_ACTIVE_USER == SAS_USER_JOHN) & FANG_ENABLE_DEV_FEATURES)

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }


//////////////////////////////////////////////////////////////////////////////////////
// Local Structures:
//////////////////////////////////////////////////////////////////////////////////////

struct _D3DColorFmtInfo_t
{
	D3DFORMAT nFormat;	// Format code (D3DFMT_UNKNOWN=end of table)

	u8 nColorBits;
	u8 nPref;			// Larger means more preferable

	u8 nRedBits;
	u8 nGreenBits;
	u8 nBlueBits;
	u8 nAlphaBits;
};

struct _D3DDepthFmtInfo_t
{
	D3DFORMAT nFormat;	// Format code (D3DFMT_UNKNOWN=end of table)

	u8 nDepthBits;		// 0=none
	u8 nStencilBits;	// 0=none
	u8 nPref;			// Larger means more preferable
};

struct _VidModeInfo_t
{
	FVidMode_t VidMode;
	D3DDISPLAYMODE DisplayMode;
	D3DFORMAT nDepthFormat;
};

struct _AdapterInfo_t
{
	FVidDev_t VidDev;

	u32 nVidModeCount;
	_VidModeInfo_t *pVidModeArray;

	u32 nD3DAdapterIndex;
};


//////////////////////////////////////////////////////////////////////////////////////
// Static variables:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _bModuleInitialized;
static FResFrame_t _MasterResFrame;
static BOOL _bBeginEndCalled;

static volatile BOOL _bBufferSwapReady[2];
static volatile BOOL _bBufferVSyncHit[2];
static u32 _nXBSwapCallbackHit;
static u32 _nXBPresentCalls;

static FVidDrawOverlayFcn_t *_pFcnDrawOverlay;

static int _nWindowCallbackCount;
static FDX8VidWindowCallback_t **_ppFcnWindowCallbackTable;
static FDX8VidMouseCallback_t *_pMouseCallback;
static void *_pMouseCallbackParm;

// Enumeration:
static BOOL _bEnumerated;
static FVidRenderer_t _nRenderer;
static D3DDEVTYPE _EnumD3DDevType;
static u32 _nAdapterArrayCount;
static _AdapterInfo_t *_pAdapterArray;

// Current video mode:
static D3DDEVTYPE _D3DDevType;
static _AdapterInfo_t _CurAdapterInfo;
static _VidModeInfo_t _CurVidModeInfo;

static D3DDISPLAYMODE _OrigDisplayMode;
static D3DADAPTER_IDENTIFIER8 _AdapterID;
static D3DCAPS8 _DeviceCaps;

static HCURSOR _hCursor;
static BOOL _bCursorOn;
static u32 _nCursorX, _nCursorY;
static BOOL _bLeftMouseButton;
static BOOL _bRightMouseButton;
static HWND _hWnd;
static BOOL _bWeCreatedTheWindow;
static DWORD _nWindowStyle;
static RECT _WindowRect;
static RECT _ClientRect;
static DWORD _nBehaviorFlags;
static DWORD _nRefreshRate;
static DWORD _nPresentationInterval;
static D3DPRESENT_PARAMETERS _PresentationParms;

static D3DGAMMARAMP _GammaRamp;

// Window operation:
static volatile BOOL _bReady;
static volatile BOOL _bDeviceLost;
static volatile BOOL _bResetFailed;
static volatile BOOL _bUserClickedClose;
static volatile BOOL _bMinimized;	// Windowed mode: TRUE indicates the window is currently minimized (always FALSE for fullscreen)
static volatile BOOL _bHaveFocus;



static const _D3DColorFmtInfo_t _aD3DColorFmtInfo[] = {
	// D3DFORMAT			Bits	Pref	A		R		G		B
#if FANG_PLATFORM_WIN
	D3DFMT_A8R8G8B8,		32,		1,		8,		8,		8,		8,
	D3DFMT_X8R8G8B8,		32,		0,		0,		8,		8,		8,

	D3DFMT_R8G8B8,			24,		0,		0,		8,		8,		8,

	D3DFMT_R5G6B5,			16,		4,		0,		5,		6,		5,
	D3DFMT_A1R5G5B5,		16,		3,		1,		5,		5,		5,
	D3DFMT_X1R5G5B5,		16,		2,		0,		5,		5,		5,
	D3DFMT_A4R4G4B4,		16,		1,		4,		4,		4,		4,
	D3DFMT_X4R4G4B4,		16,		0,		0,		4,		4,		4,
#else
	D3DFMT_LIN_A8R8G8B8,	32,		1,		8,		8,		8,		8,
	D3DFMT_LIN_R5G6B5,		16,		4,		0,		5,		6,		5,
#endif

	D3DFMT_UNKNOWN,			0,		0,		0,		0,		0,		0
};

static const _D3DDepthFmtInfo_t _aD3DDepthFmtInfo[] = {
	// D3DFORMAT			Depth	Stencil	Pref
#if FANG_PLATFORM_WIN
	D3DFMT_D32,				32,		0,		0,
#endif

	D3DFMT_D24S8,			24,		8,		0,

#if FANG_PLATFORM_WIN
	D3DFMT_D24X4S4,			24,		4,		0,
#endif

#if FANG_PLATFORM_WIN
	D3DFMT_D24X8,			24,		0,		0,
#endif

	D3DFMT_D16,				16,		0,		1,
	D3DFMT_D16_LOCKABLE,	16,		0,		0,

#if FANG_PLATFORM_WIN
	D3DFMT_D15S1,			15,		1,		0,
#endif

	D3DFMT_UNKNOWN,			0,		0,		0
};


//////////////////////////////////////////////////////////////////////////////////////
// Static Functions:
//////////////////////////////////////////////////////////////////////////////////////

static void _ClearData( void );
static void _ClearVideoData( void );
static void _Enumerate( void );
static s32  _SortVidModesCallback( const void *pArg1, const void *pArg2 );
static const _D3DColorFmtInfo_t *_FindColorFmtInfo( D3DFORMAT nFormat );
static const _D3DDepthFmtInfo_t *_FindDepthFmtInfo( D3DFORMAT nFormat );
static BOOL _CheckDepthStencilFormat( UINT nD3DAdapterIndex, D3DDEVTYPE DevType, D3DFORMAT nColorFormat, D3DFORMAT nDepthStencilFormat );
static D3DMULTISAMPLE_TYPE _GetMultiSampleType( const _AdapterInfo_t *pAdapterInfo, const _VidModeInfo_t *pVidModeInfo, f32 *pfUnitFSAA );
static DWORD _ComputePresentationInterval( u32 *pnSwapInterval, const D3DCAPS8 *pCaps );
static BOOL _SwapBuffers( void );
static BOOL _CreateWindow( void );
static void _DestroyWindow( void );
static s32  _FindWindowCallbackFunction( FDX8VidWindowCallback_t *pFcnWindowCallback );
static BOOL _CallWindowCallbackFunctions( FDX8VidEvent_e nEvent );


#if FANG_PLATFORM_WIN
	static void _SetClientSize( int nClientWidth, int nClientHeight );
	static void _RegisterWindowsClass( void );
	static void _MouseActivity( POINT *pCursorPoint, short nDeltaWheel );
	static LRESULT CALLBACK _WndProc( HWND hWnd, UINT nMsg, WPARAM nWParam, LPARAM nLParam );
	static LRESULT CALLBACK _UnSubclassD3DsWndProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
	static WNDPROC _D3DSubclassedWndProc=NULL;
#endif




//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL fvid_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	_ClearData();

	FVid_fBrightness = 1.f;

	_nWindowCallbackCount = 0;
	_ppFcnWindowCallbackTable = (FDX8VidWindowCallback_t **)fres_Alloc( sizeof(FDX8VidWindowCallback_t *) * _MAX_WINDOW_CREATION_CALLBACK_FCNS );
	if( _ppFcnWindowCallbackTable == NULL ) {
		return FALSE;
	}

	// Uncomment this if you want instantaneous GPU errors
//	D3D__SingleStepPusher = TRUE;

	_pMouseCallback = NULL;
	_pMouseCallbackParm = NULL;
	_bLeftMouseButton = FALSE;
	_bRightMouseButton = FALSE;
	fdx8vid_bResetting = FALSE;
	_bModuleInitialized = TRUE;
	_pFcnDrawOverlay = NULL;

	FVid_nSwapCounter = 0;

	return TRUE;
}


//
//
//
void fvid_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( !FVid_bOnline );
	FASSERT( _nWindowCallbackCount == 0 );	// Someone didn't unregister their window callback function!

	fvid_EnumerateReset();

	_ClearData();

	_bModuleInitialized = FALSE;
}


//
//
//
void fvid_Enumerate( FVidRenderer_t nRenderer ) 
{
	static const D3DDEVTYPE __aD3DDevTypes[FVID_RENDERER_COUNT] = { D3DDEVTYPE_HAL, D3DDEVTYPE_SW, D3DDEVTYPE_REF };

	FASSERT( _bModuleInitialized );

#if FANG_PLATFORM_XB
	nRenderer = FVID_RENDERER_HARDWARE;
#endif

	FASSERT( nRenderer>=0 && nRenderer<FVID_RENDERER_COUNT );

	if( _bEnumerated && _nRenderer==nRenderer ) 
	{
		// We have this enumeration cached. No need to enumerate again...
		return;
	}

	fvid_EnumerateReset();

	_nRenderer = nRenderer;
	_EnumD3DDevType = __aD3DDevTypes[nRenderer];

	if( FDX8_pD3D ) 
	{
		_Enumerate();
	} 
	else 
	{
		FDX8_pD3D = Direct3DCreate8( D3D_SDK_VERSION );
		_Enumerate();
		FDX8_SAFE_RELEASE( FDX8_pD3D );
	}

	_bEnumerated = TRUE;
}


//
//
//
void fvid_EnumerateReset( void ) 
{
	u32 i;

	FASSERT( _bModuleInitialized );

	if( _bEnumerated ) 
	{
		for( i=0; i<_nAdapterArrayCount; i++ ) 
		{
			fang_Free( _pAdapterArray[i].pVidModeArray );
		}

		if( _pAdapterArray ) 
		{
			fang_Free( _pAdapterArray );
			_pAdapterArray = NULL;
		}

		_nAdapterArrayCount = 0;
		_bEnumerated = FALSE;
	}
}


//
//
//
BOOL fvid_HasEnumerated( FVidRenderer_t *pnRenderer ) 
{
	FASSERT( _bModuleInitialized );

	if( pnRenderer ) 
	{
		*pnRenderer = _nRenderer;
	}

	return _bEnumerated;
}


//
//
//
u32 fvid_GetDeviceCount( void ) 
{
	FASSERT( _bModuleInitialized );

	if( _bEnumerated ) 
	{
		return _nAdapterArrayCount;
	} 
	else 
	{
		return 0;
	}
}


//
//
//
const FVidDev_t *fvid_GetDeviceInfo( u32 nDevIndex ) 
{
	FASSERT( _bModuleInitialized );

	if( _bEnumerated ) 
	{
		return (nDevIndex < _nAdapterArrayCount) ? &_pAdapterArray[nDevIndex].VidDev : NULL;
	} 
	else 
	{
		return NULL;
	}
}


//
//
//
u32 fvid_GetModeCount( u32 nDevIndex ) 
{
	FASSERT( _bModuleInitialized );

	if( _bEnumerated ) 
	{
		return (nDevIndex < _nAdapterArrayCount) ? _pAdapterArray[nDevIndex].nVidModeCount : 0;
	} 
	else 
	{
		return 0;
	}
}


//
//
//
const FVidMode_t *fvid_GetModeInfo( u32 nDevIndex, u32 nModeIndex ) 
{
	FASSERT( _bModuleInitialized );

	if( _bEnumerated ) 
	{
		if( nDevIndex>=_nAdapterArrayCount || nModeIndex>=_pAdapterArray[nDevIndex].nVidModeCount ) 
		{
			return NULL;
		}

		return &_pAdapterArray[nDevIndex].pVidModeArray[nModeIndex].VidMode;
	} 
	else 
	{
		return NULL;
	}
}


//
//
//
FVidDrawOverlayFcn_t *fvid_GetDrawOverlayFcn( void ) 
{
	FASSERT( _bModuleInitialized );
	return _pFcnDrawOverlay;
}


//
//
//
void fvid_SetDrawOverlayFcn( FVidDrawOverlayFcn_t *pFcnDrawOverlay ) 
{
	FASSERT( _bModuleInitialized );
	_pFcnDrawOverlay = pFcnDrawOverlay;
}


//
//
//
BOOL fvid_CreateWindow( const FVidWin_t *pWinInfo ) 
{
	_AdapterInfo_t *pCurAdapterInfo;
	_VidModeInfo_t *pCurVidModeInfo;
	u32 i;

	FASSERT( _bModuleInitialized );

	FASSERT( pWinInfo->VidDev.nRenderer>=0 && pWinInfo->VidDev.nRenderer<FVID_RENDERER_COUNT );
	FASSERT( pWinInfo->nSwapInterval==0 || pWinInfo->nSwapInterval==1 || pWinInfo->nSwapInterval==2 );
	FASSERT_UNIT_FLOAT( pWinInfo->fUnitFSAA );

	fvid_DestroyWindow();
	_pFcnDrawOverlay = NULL;

	// Have we enumerated the desired Renderer?
	if( !_bEnumerated || _nRenderer!=(FVidRenderer_t)pWinInfo->VidDev.nRenderer ) 
	{
		fvid_Enumerate( (FVidRenderer_t)pWinInfo->VidDev.nRenderer );
	}

	// Attempt to locate the specified device in the enumeration table...
	for( i=0; i<_nAdapterArrayCount; i++ ) 
	{
		pCurAdapterInfo = &_pAdapterArray[i];

		if( !strcmp( pCurAdapterInfo->VidDev.szName, pWinInfo->VidDev.szName ) ) 
		{
			// Names are identical. Check the ordinal...

			if( pCurAdapterInfo->VidDev.nOrdinal == pWinInfo->VidDev.nOrdinal ) 
			{
				// Ordinals are identical. Check the flags...
				if( pCurAdapterInfo->VidDev.nFlags == pWinInfo->VidDev.nFlags ) 
				{
					// This is the matching adapter...
					break;
				}
			}
		}
	}
	if( i == _nAdapterArrayCount ) 
	{
		// Adapter not found...
		//return FALSE;

		pCurAdapterInfo = &_pAdapterArray[0];
	}

	// Attempt to locate the specified mode in the enumeration table...
	for( i=0; i<pCurAdapterInfo->nVidModeCount; i++ ) 
	{
		pCurVidModeInfo = &pCurAdapterInfo->pVidModeArray[i];

		if( pCurVidModeInfo->VidMode.nFlags == pWinInfo->VidMode.nFlags ) 
		{
			if( pCurVidModeInfo->VidMode.nColorBits == pWinInfo->VidMode.nColorBits ) 
			{
				if( pCurVidModeInfo->VidMode.nDepthBits == pWinInfo->VidMode.nDepthBits ) 
				{
					if( pCurVidModeInfo->VidMode.nStencilBits == pWinInfo->VidMode.nStencilBits ) 
					{
						if( pCurVidModeInfo->VidMode.nPixelsAcross == pWinInfo->VidMode.nPixelsAcross ) 
						{
							if( pCurVidModeInfo->VidMode.nPixelsDown == pWinInfo->VidMode.nPixelsDown ) 
							{
								// This is the matching mode...
								break;
							}
						}
					}
				}
			}
		}
	}
	if( i == pCurAdapterInfo->nVidModeCount ) 
	{
		// Mode not found...
		//return FALSE;
		pCurVidModeInfo = &pCurAdapterInfo->pVidModeArray[0];
	}

	_D3DDevType = _EnumD3DDevType;

	_CurAdapterInfo = *pCurAdapterInfo;
	_CurVidModeInfo = *pCurVidModeInfo;

	_CurAdapterInfo.nVidModeCount = 0;
	_CurAdapterInfo.pVidModeArray = NULL;

	FVid_Win = *pWinInfo;
	FVid_Dev = _CurAdapterInfo.VidDev;
	FVid_Mode = _CurVidModeInfo.VidMode;

	FDX8_pD3D = Direct3DCreate8( D3D_SDK_VERSION );

	if( _CreateWindow() ) 
	{
		fdx8_InitD3DState();
		if( _CallWindowCallbackFunctions( FDX8VID_EVENT_WINDOW_CREATED ) ) 
		{
			return TRUE;
		}

		_DestroyWindow();

		return FALSE;
	} 
	else 
	{
		FDX8_SAFE_RELEASE( FDX8_pD3D );
		return FALSE;
	}
}


//
//
//
void fvid_DestroyWindow( void ) 
{
	FASSERT( _bModuleInitialized );

	if( FVid_bOnline ) 
	{
		_CallWindowCallbackFunctions( FDX8VID_EVENT_WINDOW_DESTROYED );

		if( FVid_bBegin ) 
		{
			fvid_End();
		}

		_DestroyWindow();
	}
}


//
//
//
BOOL fvid_IsMinimized( void ) 
{
	FASSERT( _bModuleInitialized );

	if( FVid_bOnline ) 
	{
		return _bMinimized;
	} 
	else 
	{
		return FALSE;
	}
}


//
//
//
BOOL fvid_HasUserClosedWindow( void ) 
{
	FASSERT( _bModuleInitialized );

	if( FVid_bOnline ) 
	{
		return _bUserClickedClose;
	}

	return FALSE;
}


//
//
//
BOOL fvid_Begin( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );
	FASSERT( !FVid_bBegin );

	FVid_bBegin = TRUE;

	fcolor_GenerateMotifs();
	fvid_IncrementFrameCounter();
	fmesh_ResetLightList();

	FDX8_pDev->BeginScene();

	// If the graphics window is minimized or if the device is lost,
	// there's no need to have the game render anything...
	return !(_bMinimized || _bDeviceLost);
}


//
//
//
void fvid_End( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );

	if( FVid_bBegin ) 
	{
		_bBeginEndCalled = TRUE;

		fperf_RenderPerf();

		#if !FANG_PRODUCTION_BUILD && !FANG_DEBUG_BUILD
			if ( FPerf_nDisplayPerfType != FPERF_TYPE_NONE )
			{
				MEMORYSTATUS stat;
				GlobalMemoryStatus( &stat );
				if ( ((u32)FLoop_nTotalLoopSecs % 15) < 2 )
				{
					if ( FRes_CFHeap.GetFreeBytes() <= 0 || ((s32)stat.dwAvailPhys - (s32)(64 * 1024 * 1024)) <= 0 )
					{
						ftext_DebugPrintf( 0.5f, 0.3f, "~s1.50~c99111199~w1~acCURRENTLY EXCEEDING MEMORY!!!" );
					}
				}
			}
		#endif

//	to spew the heap status and allocations to the stdo
//		if (FVid_nSwapCounter == 250)
//		{
//			fheap_ShowMemoryTracking();
//		}
//
		ftext_Draw();

		if( _pFcnDrawOverlay ) {
			_pFcnDrawOverlay();
		}

		FVid_bBegin = FALSE;

		FDX8_pDev->EndScene();
	}
}


//
//
//
#if FANG_WINGC
volatile LONG FVid_nPortSwapTick, FVid_nPortGameThreadId;
#endif

BOOL fvid_Swap( void )
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );
	FASSERT( !FVid_bBegin );

//	_fsound_Update();

	frenderer_PopAll();

#if FANG_WINGC
	// Port diagnostics: frames that took far longer than the running average (PORT-HITCH), and every
	// 10 seconds the frame rate, the worst frame and the time spent before presenting (PORT-PERF; run
	// with -no-vsync to see the work time as the frame time).
	static LARGE_INTEGER _nPerfFreq, _nPresentEnd;
	if( Fang_bPortDiag )
	{
		// for main_win.cpp's stall watchdog: when the game thread last swapped, and which thread it is
		InterlockedExchange( &FVid_nPortSwapTick, (LONG)GetTickCount() );
		FVid_nPortGameThreadId = (LONG)GetCurrentThreadId();
		static LARGE_INTEGER _nLast, _nWindowStart;
		static f32 _fAvgMs = 16.0f;
		static u32 _nFrames = 0, _nWindowFrames = 0;
		static f64 _fWindowWorkMs = 0.0, _fWindowWorstMs = 0.0;
		LARGE_INTEGER nNow;
		QueryPerformanceCounter( &nNow );
		if( !_nPerfFreq.QuadPart ) QueryPerformanceFrequency( &_nPerfFreq );
		const f64 fMsPerTick = 1000.0 / (f64)_nPerfFreq.QuadPart;
		if( _nLast.QuadPart )
		{
			f32 fMs = (f32)( (f64)( nNow.QuadPart - _nLast.QuadPart ) * fMsPerTick );
			if( ++_nFrames > 60 && fMs > 40.0f && fMs > 2.5f * _fAvgMs )
			{
				DEVPRINTF( "PORT-HITCH frame %u: %.1f ms (average %.1f ms), t=%.2f s\n", FVid_nFrameCounter, fMs, _fAvgMs, (f32)( (f64)nNow.QuadPart / (f64)_nPerfFreq.QuadPart ) );
			}
			_fAvgMs += ( fMs - _fAvgMs ) * 0.05f;

			_nWindowFrames++;
			if( _nPresentEnd.QuadPart ) _fWindowWorkMs += (f64)( nNow.QuadPart - _nPresentEnd.QuadPart ) * fMsPerTick;
			if( fMs > _fWindowWorstMs ) _fWindowWorstMs = fMs;
			const f64 fWindowMs = (f64)( nNow.QuadPart - _nWindowStart.QuadPart ) * fMsPerTick;
			if( fWindowMs >= 10000.0 )
			{
				DEVPRINTF( "PORT-PERF %u frames in %.1f s: %.1f fps, frame %.2f ms average, worst %.1f ms, work before present %.2f ms average\n",
					_nWindowFrames, fWindowMs / 1000.0, _nWindowFrames * 1000.0 / fWindowMs, fWindowMs / _nWindowFrames,
					_fWindowWorstMs, _fWindowWorkMs / _nWindowFrames );
				_nWindowStart = nNow;
				_nWindowFrames = 0;
				_fWindowWorkMs = _fWindowWorstMs = 0.0;
			}
		}
		else
		{
			_nWindowStart = nNow;
		}
		_nLast = nNow;
	}
#endif

	if( _bBeginEndCalled )
	{
		_bBeginEndCalled = FALSE;
#if FANG_WINGC
		const BOOL bSwapped = _SwapBuffers();
		if( Fang_bPortDiag )
		{
			QueryPerformanceCounter( &_nPresentEnd );
		}
		return bSwapped;
#else
		return _SwapBuffers();
#endif
	}
	else 
	{
		DEVPRINTF( "fvid_Swap(): Begin/End vid functions have not been called. Cannot render.\n" );
		_bBeginEndCalled = FALSE;
		return TRUE;
	}
}


//
//
//
void fvid_Flush( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );
	FASSERT( !FVid_bBegin );

	// No implementation necessary for DX8.
}


//
//
//
void fvid_ResetFrameCounter( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );

	FVid_nFrameCounter = 1;
	FVid_bPrintHiFreqErrMsg = FALSE;
}


//
//
//
void fvid_IncrementFrameCounter( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );

	FVid_nFrameCounter++;

	FVid_bPrintHiFreqErrMsg = FALSE;
	if( !(FVid_nFrameCounter & 63) ) 
	{
		FVid_bPrintHiFreqErrMsg = TRUE;
	} 
}


//
// Returns the size of the screen-grab buffer, in bytes.
//
u32 fvid_GetScreenGrabBufferSizeInBytes( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );

	return (FVid_Mode.nPixelsAcross * FVid_Mode.nPixelsDown * sizeof(u32) );
}


//
// Stores the contects of the front buffer into the pDestBuffer.
// Each pixel will be converted to A8R8G8B8 format (char[0]=blue, char[1]=green, char[2]=red, char[3]=alpha)
//
BOOL fvid_ScreenGrab( void *pDestBuffer, u32 nBufferBytes ) 
{
#if FANG_PLATFORM_XB
	// TODO
	return FALSE;
#else
	IDirect3DSurface8 *pSurface;
	D3DLOCKED_RECT LockedRect;
	u32 nScreenGrabBytes;

	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );

	nScreenGrabBytes = FVid_Mode.nPixelsAcross * FVid_Mode.nPixelsDown * sizeof(u32);

	if( nBufferBytes < nScreenGrabBytes ) 
	{
		DEVPRINTF( "fvid_ScreenGrab(): Insufficient buffer size specified: %u (%u needed)\n", nBufferBytes, nScreenGrabBytes );
		return FALSE;
	}

	if( FAILED( FDX8_pDev->CreateImageSurface( FVid_Mode.nPixelsAcross, FVid_Mode.nPixelsDown, D3DFMT_A8R8G8B8, &pSurface ) ) ) 
	{
		DEVPRINTF( "fvid_ScreenGrab(): Could not create temporary D3D surface.\n" );
		return FALSE;
	}

	if( FAILED( FDX8_pDev->GetFrontBuffer( pSurface ) ) ) 
	{
		pSurface->Release();
		DEVPRINTF( "fvid_ScreenGrab(): Could not grab screen.\n" );
		return FALSE;
	}

	pSurface->LockRect( &LockedRect, NULL, D3DLOCK_READONLY );

	fang_MemCopy( pDestBuffer, LockedRect.pBits, nScreenGrabBytes );

	pSurface->UnlockRect();
	pSurface->Release();

	return TRUE;
#endif
}


//
// fUnitGamma can range from 0.0f to 1.0f (0.5f is default)
//
void fvid_SetGamma( f32 fUnitGamma ) 
{
	int i, nNormalGamma, nGammaExtent, nGamma;
	f32 fInterp, fVal;
	D3DGAMMARAMP GammaRamp;

	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );
	FASSERT_UNIT_FLOAT( fUnitGamma );

	if( fUnitGamma >= 0.5f ) {
		fInterp = (fUnitGamma - 0.5f) * 2.0f;
	} else {
		fInterp = 1.0f - (fUnitGamma * 2.0f);
	}

	for( i=0, nNormalGamma=0; i<256; i++, nNormalGamma+=256 ) {
		if( fUnitGamma >= 0.5f ) {
			// Calculate the max gamma...
			fVal = (f32)(256 - i);
			nGammaExtent = 65536 - (int)( fVal * fVal * fVal * 0.00390625f );
		} else {
			// Calculate the min gamma...
			fVal = (f32)i;
			nGammaExtent = (int)( fVal * fVal * fVal * 0.00390625f );
		}

		// Interpolate between normal gamma and the extent that was calculated...
		nGamma = nNormalGamma + (int)( (f32)(nGammaExtent - nNormalGamma) * fInterp );
		FMATH_CLAMP( nGamma, 0, 65535 );

		// Set the D3D ramp...
		GammaRamp.red[i] = nGamma;
		GammaRamp.green[i] = nGamma;
		GammaRamp.blue[i] = nGamma;
	}

	FDX8_pDev->SetGammaRamp( D3DSGR_NO_CALIBRATION, &GammaRamp );
}


//
//
//
void fdx8vid_RegisterWindowCallbackFunction( FDX8VidWindowCallback_t *pFcnWindowCallback ) 
{
	FASSERT( _bModuleInitialized );

	if( pFcnWindowCallback == NULL ) 
	{
		return;
	}

	FASSERT( _FindWindowCallbackFunction( pFcnWindowCallback ) == -1 );
	FASSERT( _nWindowCallbackCount < _MAX_WINDOW_CREATION_CALLBACK_FCNS );

	_ppFcnWindowCallbackTable[_nWindowCallbackCount++] = pFcnWindowCallback;
}


//
//
//
void fdx8vid_UnregisterWindowCallbackFunction( FDX8VidWindowCallback_t *pFcnWindowCallback ) 
{
	int i, nIndex;

	FASSERT( _bModuleInitialized );

	if( pFcnWindowCallback == NULL ) {
		return;
	}

	nIndex = _FindWindowCallbackFunction( pFcnWindowCallback );

	if( nIndex != -1 ) {
		// Function found...

		for( i=nIndex+1; i<_nWindowCallbackCount; i++ ) {
			_ppFcnWindowCallbackTable[i-1] = _ppFcnWindowCallbackTable[i];
		}

		_nWindowCallbackCount--;
	}
}


//
//
//
HWND fdx8vid_GetWindowHandle( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );
	return _hWnd;
}


//
//
//
BOOL fdx8vid_IsCursorEnabled( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );
	return _bCursorOn;
}


//
//
//
void fdx8vid_EnableCursor( BOOL bEnable ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );
	_bCursorOn = bEnable;

	if( FVid_Mode.nFlags & FVID_MODEFLAG_WINDOWED ) 
	{
		if( _bCursorOn ) 
		{
#			if FANG_PLATFORM_WIN
				SetCursor( _hCursor );
#			endif
		} 
		else 
		{
#			if FANG_PLATFORM_WIN
				SetCursor( NULL );
#			endif
		}
	} 
	else 
	{
#		if FANG_PLATFORM_WIN
			FDX8_pDev->ShowCursor( _bCursorOn );
#		endif
	}
}


//
//
//
void fdx8vid_SetCursorPosition( u32 nScreenspaceX, u32 nScreenspaceY ) 
{
	POINT CursorPoint;

	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );

	_nCursorX = nScreenspaceX;
	_nCursorY = nScreenspaceY;

	CursorPoint.x = _nCursorX;
	CursorPoint.y = _nCursorY;
#	if FANG_PLATFORM_WIN
		ClientToScreen( _hWnd, &CursorPoint );
		SetCursorPos( CursorPoint.x, CursorPoint.y );
#	endif
}


//
//
//
void fdx8vid_GetCursorPosition( u32 *pnScreenspaceX, u32 *pnScreenspaceY ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );

	if( pnScreenspaceX ) 
	{
		*pnScreenspaceX = _nCursorX;
	}
	if( pnScreenspaceY ) 
	{
		*pnScreenspaceY = _nCursorY;
	}
}


//
//
//
BOOL fdx8vid_IsWindowCallbackFunctionRegistered( FDX8VidWindowCallback_t *pFcnWindowCallback ) 
{
	FASSERT( _bModuleInitialized );

	if( pFcnWindowCallback == NULL ) 
	{
		return TRUE;
	}

	if( _FindWindowCallbackFunction( pFcnWindowCallback ) != -1 ) 
	{
		return TRUE;
	} 
	else 
	{
		return FALSE;
	}
}


//
//
//
BOOL fdx8vid_CheckDeviceFormat( DWORD nUsageFlags, D3DRESOURCETYPE nResourceType, D3DFORMAT nFormatToCheck ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );

	if( SUCCEEDED( FDX8_pD3D->CheckDeviceFormat(
						_CurAdapterInfo.nD3DAdapterIndex,
						_D3DDevType,
						_CurVidModeInfo.DisplayMode.Format,
						nUsageFlags,
						nResourceType,
						nFormatToCheck
						) ) ) 
	{
		return TRUE;
	}

	return FALSE;
}


//
//
//
BOOL fdx8vid_CheckDepthStencilMatch( D3DFORMAT nColorFormat, D3DFORMAT nDepthStencilFormat ) 
{
	HRESULT nResult;

	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );

	nResult = FDX8_pD3D->CheckDepthStencilMatch(
						_CurAdapterInfo.nD3DAdapterIndex,
						_D3DDevType,
						_CurVidModeInfo.DisplayMode.Format,
						nColorFormat,
						nDepthStencilFormat
					);

	if( nResult == D3D_OK ) 
	{
		return TRUE;
	}

	return FALSE;
}


//
//
//
BOOL fdx8vid_SetCursor( HCURSOR hCursor ) 
{
#if FANG_PLATFORM_WIN
	HRESULT hr;
	ICONINFO IconInfo;
	LPDIRECT3DSURFACE8 pCursorBitmap = NULL;
	HDC ColorHDC, MaskHDC, ScreenHDC;
	BITMAP BitMap;
	DWORD nWidth, nHeightSrc, nHeightDest, *pnBitmap;
	COLORREF ColorRef, ColorMask;
	UINT x, y;
	BITMAPINFO BitMapInfo;
	COLORREF *pArrayColor, *pArrayMask;
	HGDIOBJ hGDIObjOld;
	BOOL bBWCursor, bSuccess;

	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );

	_hCursor = 0;

	if( FVid_Mode.nFlags & FVID_MODEFLAG_WINDOWED ) 
	{
		_hCursor = hCursor;
		SetClassLong( _hWnd, GCL_HCURSOR, (long)hCursor );
		return TRUE;
	}

	bSuccess = FALSE;
	ColorHDC = NULL;
	MaskHDC = NULL;
	ScreenHDC = NULL;
	pArrayColor = NULL;
	pArrayMask = NULL;

	ZeroMemory( &IconInfo, sizeof(IconInfo) );
	if( !GetIconInfo( hCursor, &IconInfo ) ) 
	{
		goto _ExitSetCursor;
	}

	if( !GetObject( (HGDIOBJ)IconInfo.hbmMask, sizeof(BITMAP), (LPVOID)&BitMap ) ) 
	{
		goto _ExitSetCursor;
	}

	nWidth = BitMap.bmWidth;
	nHeightSrc = BitMap.bmHeight;

	if( IconInfo.hbmColor == NULL ) 
	{
		bBWCursor = TRUE;
		nHeightDest = nHeightSrc >> 1;
	} 
	else 
	{
		bBWCursor = FALSE;
		nHeightDest = nHeightSrc;
	}

	// Create a surface for the fullscreen cursor...
	if( FAILED( hr = FDX8_pDev->CreateImageSurface( nWidth, nHeightDest, D3DFMT_A8R8G8B8, &pCursorBitmap ) ) ) 
	{
		goto _ExitSetCursor;
	}

	pArrayMask = new DWORD[nWidth * nHeightSrc];
	if( pArrayMask == NULL ) 
	{
		goto _ExitSetCursor;
	}

	ZeroMemory( &BitMapInfo, sizeof(BitMapInfo) );
	BitMapInfo.bmiHeader.biSize = sizeof(BitMapInfo.bmiHeader);
	BitMapInfo.bmiHeader.biWidth = nWidth;
	BitMapInfo.bmiHeader.biHeight = nHeightSrc;
	BitMapInfo.bmiHeader.biPlanes = 1;
	BitMapInfo.bmiHeader.biBitCount = 32;
	BitMapInfo.bmiHeader.biCompression = BI_RGB;

	ScreenHDC = GetDC( NULL );
	MaskHDC = CreateCompatibleDC( ScreenHDC );
	if( MaskHDC == NULL ) 
	{
		goto _ExitSetCursor;
	}

	hGDIObjOld = SelectObject( MaskHDC, IconInfo.hbmMask );
	GetDIBits( MaskHDC, IconInfo.hbmMask, 0, nHeightSrc, pArrayMask, &BitMapInfo, DIB_RGB_COLORS );
	SelectObject( MaskHDC, hGDIObjOld );

	if( !bBWCursor ) 
	{
		pArrayColor = new DWORD[nWidth * nHeightDest];
		ColorHDC = CreateCompatibleDC( GetDC( NULL ) );
		if( ColorHDC == NULL ) 
		{
			goto _ExitSetCursor;
		}
		SelectObject( ColorHDC, IconInfo.hbmColor );
		GetDIBits( ColorHDC, IconInfo.hbmColor, 0, nHeightDest, pArrayColor, &BitMapInfo, DIB_RGB_COLORS );
	}

	// Transfer cursor image into the surface...
	D3DLOCKED_RECT LockedRect;
	pCursorBitmap->LockRect( &LockedRect, NULL, 0 );
	pnBitmap = (DWORD*)LockedRect.pBits;
	for( y = 0; y<nHeightDest; y++ ) 
	{
		for( x = 0; x<nWidth; x++ ) 
		{
			if( bBWCursor ) 
			{
				ColorRef = pArrayMask[nWidth*(nHeightDest-1-y) + x];
				ColorMask = pArrayMask[nWidth*(nHeightSrc-1-y) + x];
			} 
			else 
			{
				ColorRef = pArrayColor[nWidth*(nHeightDest-1-y) + x];
				ColorMask = pArrayMask[nWidth*(nHeightDest-1-y) + x];
			}

			if( ColorMask == 0 ) 
			{
				pnBitmap[nWidth*y + x] = 0xff000000 | ColorRef;
			} 
			else 
			{
				pnBitmap[nWidth*y + x] = 0x00000000;
			}
		}
	}
	pCursorBitmap->UnlockRect();

	// Set the device cursor...
	if( FAILED( hr = FDX8_pDev->SetCursorProperties( IconInfo.xHotspot, IconInfo.yHotspot, pCursorBitmap ) ) ) 
	{
		goto _ExitSetCursor;
	}

	_hCursor = hCursor;
	bSuccess = TRUE;

_ExitSetCursor:
	if( IconInfo.hbmMask != NULL ) 
	{
		DeleteObject( IconInfo.hbmMask );
	}

	if( IconInfo.hbmColor != NULL ) 
	{
		DeleteObject( IconInfo.hbmColor );
	}

	if( ScreenHDC != NULL ) 
	{
		ReleaseDC( NULL, ScreenHDC );
	}

	if( ColorHDC != NULL ) 
	{
		DeleteDC( ColorHDC );
	}

	if( MaskHDC != NULL ) 
	{
		DeleteDC( MaskHDC );
	}

	SAFE_DELETE_ARRAY( pArrayColor );
	SAFE_DELETE_ARRAY( pArrayMask );
	FDX8_SAFE_RELEASE( pCursorBitmap );

	return bSuccess;
#else
	return TRUE;
#endif
}


//
//
//
void fdx8vid_MinimizeWindow( void ) 
{
#if FANG_PLATFORM_WIN
	WINDOWPLACEMENT Placement;

	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );

	Placement.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement( _hWnd, &Placement );

	Placement.length = sizeof(WINDOWPLACEMENT);
	Placement.flags = 0;
	Placement.showCmd = SW_MINIMIZE;
	SetWindowPlacement( _hWnd, &Placement );
#endif
}


//
//
//
void fdx8vid_RestoreWindow( void ) 
{
#if FANG_PLATFORM_WIN
	WINDOWPLACEMENT Placement;

	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );

	Placement.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement( _hWnd, &Placement );

	Placement.length = sizeof(WINDOWPLACEMENT);
	Placement.flags = 0;
	Placement.showCmd = SW_RESTORE;
	SetWindowPlacement( _hWnd, &Placement );

	while( _bDeviceLost ) 
	{
		if( !_SwapBuffers() ) 
		{
			break;
		}
	}

	SetForegroundWindow( _hWnd );
#endif
}


//
//
//
BOOL fdx8vid_IsDeviceLost( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );
	return _bDeviceLost;
}


//
//
//
BOOL fdx8vid_HaveFocus( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );
	return _bHaveFocus && !_bMinimized;
}


//
//
//
void fdx8vid_RegisterMouseCallback( FDX8VidMouseCallback_t *pMouseCallback, void *pParm ) 
{
	FASSERT( _bModuleInitialized );
	_pMouseCallback = pMouseCallback;
	_pMouseCallbackParm = pParm;
}


//
//
//
FDX8VidMouseCallback_t *fdx8vid_GetMouseCallback( void ) 
{
	FASSERT( _bModuleInitialized );
	return _pMouseCallback;
}


//
//
//
void *fdx8vid_GetMouseCallbackParm( void ) 
{
	FASSERT( _bModuleInitialized );
	return _pMouseCallbackParm;
}


//
//
//
static void _ClearData( void ) 
{
	FVid_bOnline = FALSE;

	FDX8_pD3D = NULL;
	FDX8_pDev = NULL;
	FDX8_pSurfBack = NULL;
	FDX8_pSurfDepthStencil = NULL;

	_bEnumerated = FALSE;
	_nAdapterArrayCount = 0;
	_pAdapterArray = NULL;

	_ClearVideoData();
}


//
//
//
static void _ClearVideoData( void ) 
{
	FANG_ZEROSTRUCT( FVid_Dev );
	FANG_ZEROSTRUCT( FVid_Mode );
	FANG_ZEROSTRUCT( FVid_Win );

	_bWeCreatedTheWindow = FALSE;
	_hWnd = NULL;
	_bMinimized = FALSE;

	_bBeginEndCalled = FALSE;

	FVid_bBegin = FALSE;
	FVid_nFrameCounter = 0;
	FVid_bPrintHiFreqErrMsg = FALSE;
}


//
//
//
static void _Enumerate( void ) 
{
	u32 nD3DMaxAdapters, nD3DMaxModes, nModeCount, nCFormatCount, nWindowResTableCount, nValidWindowResCount;
	u32 m, f, d, i, j;
	_AdapterInfo_t *pAdapterInfo;
	_VidModeInfo_t *pVidModeArray;
	UINT nD3DAdapterIndex, nD3DModeIndex;
	D3DDISPLAYMODE OrigDisplayMode;
	D3DADAPTER_IDENTIFIER8 AdapterID;
	D3DCAPS8 DeviceCaps;
	D3DDISPLAYMODE aMode[_MAX_D3D_MODES_PER_ADAPTER];
	D3DFORMAT aCFormat[_MAX_D3D_FORMATS_PER_ADAPTER];
	D3DFORMAT aDSFormat[_MAX_D3D_FORMATS_PER_ADAPTER][_MAX_D3D_FORMATS_PER_ADAPTER];
	u32 anDSFormatCount[_MAX_D3D_FORMATS_PER_ADAPTER];
	BOOL bScanningFullscreen;

#if FANG_PLATFORM_WIN
	static const D3DFORMAT __aDSFormat[] = { D3DFMT_D32, D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_D24X8, D3DFMT_D16, D3DFMT_D15S1, D3DFMT_UNKNOWN };
#else
	static const D3DFORMAT __aDSFormat[] = { D3DFMT_D24S8, D3DFMT_D16, D3DFMT_UNKNOWN };
#endif

	static const struct { u16 nWidth, nHeight; } __aWindowResArray[] = {
		320,	200,
		320,	240,
		400,	300,
		480,	360,
		512,	384,
		640,	400,
		640,	480,
		720,	480,
		720,	576,
		800,	600,
		960,	720,
		1024,	768,
		1152,	864,
		1280,	720,
		1280,	960,
		1280,	1024,
		1600,	900,
		1600,	1200,
		1920,	1080,
		1920,	1200,
		1920,	1440,
		2048,	1536,
		0,0
	};

	if( FDX8_pD3D == NULL ) 
	{
		// D3D8 object could not be obtained...
		return;
	}

	// Count entries in __aWindowResArray[]...
	for( nWindowResTableCount=0; __aWindowResArray[nWindowResTableCount].nWidth!=0; nWindowResTableCount++ );

	_nAdapterArrayCount = 0;

	nD3DMaxAdapters = FDX8_pD3D->GetAdapterCount();

	_pAdapterArray = (_AdapterInfo_t *)fang_Malloc( nD3DMaxAdapters * sizeof(_AdapterInfo_t), 0 );
	if( _pAdapterArray == NULL ) 
	{
		// Not enough memory. Exit reporting no enumerated devices...
		return;
	}

	pVidModeArray = (_VidModeInfo_t *)fang_Malloc( _MAX_VIDMODES * sizeof(_VidModeInfo_t), 0 );
	if( pVidModeArray == NULL ) 
	{
		// Not enough memory...
		fang_Free( _pAdapterArray );
		return;
	}

	for( nD3DAdapterIndex=0; nD3DAdapterIndex<nD3DMaxAdapters; nD3DAdapterIndex++ ) 
	{
		pAdapterInfo = &_pAdapterArray[_nAdapterArrayCount];

		pAdapterInfo->nD3DAdapterIndex = nD3DAdapterIndex;
		pAdapterInfo->VidDev.nFlags = 0;
		pAdapterInfo->VidDev.nOrdinal = 0;
		pAdapterInfo->VidDev.nRenderer = (u8)_nRenderer;

		if( FAILED( FDX8_pD3D->GetAdapterIdentifier( nD3DAdapterIndex, 0, &AdapterID ) ) ) 
		{
			// Could not get adapter ID. Skip this adapter...
			continue;
		}

#if FANG_PLATFORM_WIN
		if( FDX8_pD3D->GetAdapterDisplayMode( nD3DAdapterIndex, &OrigDisplayMode ) != D3D_OK ) 
		{
			// Trouble obtaining the current display mode. Skip this adapter...
			continue;
		}
#else
		fang_MemZero( &OrigDisplayMode, sizeof(OrigDisplayMode) );
		nValidWindowResCount = 0;
#endif

		if( FAILED( FDX8_pD3D->GetDeviceCaps( nD3DAdapterIndex, _EnumD3DDevType, &DeviceCaps ) ) ) 
		{
			// Could not retrieve device caps. Skip this adapter...
			continue;
		}

		// Copy device name into public structure...
		strncpy( pAdapterInfo->VidDev.szName, AdapterID.Description, FVID_DEVNAME_LEN );
		pAdapterInfo->VidDev.szName[FVID_DEVNAME_LEN] = 0;

		// Remove trailing spaces from adapter name...
		s32 nStringPos = fclib_strlen( pAdapterInfo->VidDev.szName ) - 1;
		for( ; nStringPos >= 0; --nStringPos ) 
		{
			if( pAdapterInfo->VidDev.szName[nStringPos] != ' ' ) 
			{
				pAdapterInfo->VidDev.szName[nStringPos+1] = 0;
				break;
			}
		}

		// Set ordinal...
		for( i=0; i<nD3DAdapterIndex; i++ ) 
		{
			if( !strcmp( pAdapterInfo->VidDev.szName, _pAdapterArray[i].VidDev.szName ) ) 
			{
				// Names are identical. Increment ordinal...
				pAdapterInfo->VidDev.nOrdinal++;
			}
		}

		if( DeviceCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) 
		{
			pAdapterInfo->VidDev.nFlags |= FVID_DEVFLAG_HW_TNL;
		}

#if FANG_PLATFORM_WIN
		// Count the number of valid window resolutions...
		nValidWindowResCount = 0;
		for( i=0; i<nWindowResTableCount; i++ ) 
		{
			if( __aWindowResArray[i].nWidth  < OrigDisplayMode.Width &&
				__aWindowResArray[i].nHeight < OrigDisplayMode.Height ) 
			{
				nValidWindowResCount++;
			}
		}
#endif

		nModeCount = 0;
		nCFormatCount = 0;

		FASSERT( FANG_PLATFORM_XB==0 || FANG_PLATFORM_XB==1 );
		for( bScanningFullscreen=FANG_PLATFORM_XB; bScanningFullscreen<2; bScanningFullscreen++ ) 
		{
			if( FALSE && !bScanningFullscreen && !(DeviceCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED) ) 
			{
				// This adapter cannot render to a window, so don't enumerate window modes...
				continue;
			}

			if( bScanningFullscreen ) 
			{
				nD3DMaxModes = FDX8_pD3D->GetAdapterModeCount( nD3DAdapterIndex );
				if( nD3DMaxModes == 0 ) 
				{
					// No modes for this adapter. Skip it...
					continue;
				}
			} 
			else 
			{
				FASSERT( !FANG_PLATFORM_XB );
				nD3DMaxModes = nValidWindowResCount;
#if FANG_WINGC
				nD3DMaxModes++;	// plus the desktop's own size (a borderless window covering it)
#endif
			}

			for( nD3DModeIndex=0; nD3DModeIndex<nD3DMaxModes; nD3DModeIndex++ ) 
			{
				D3DDISPLAYMODE DisplayMode;

				if( nModeCount >= _MAX_D3D_MODES_PER_ADAPTER ) 
				{
					// Too many modes....
					break;
				}

				if( bScanningFullscreen ) 
				{
					if( FAILED( FDX8_pD3D->EnumAdapterModes( nD3DAdapterIndex, nD3DModeIndex, &DisplayMode ) ) ) 
					{
						// Could not enumerate this adapter mode. Skip it...
						continue;
					}

					DisplayMode.RefreshRate = 0;	// Note: we cheat and use RefreshRate as bWindowed
				} 
				else 
				{
					FASSERT( !FANG_PLATFORM_XB );

#if FANG_WINGC
					if( nD3DModeIndex == nValidWindowResCount ) {
						DisplayMode.Width  = OrigDisplayMode.Width;
						DisplayMode.Height = OrigDisplayMode.Height;
						DisplayMode.Format = OrigDisplayMode.Format;
						DisplayMode.RefreshRate = 1;	// Note: we cheat and use RefreshRate as bWindowed
					}
#endif
					j = 0;
					for( i=0; i<nWindowResTableCount; i++ ) 
					{
						if( __aWindowResArray[i].nWidth  < OrigDisplayMode.Width &&
							__aWindowResArray[i].nHeight < OrigDisplayMode.Height ) 
						{

							if( j++ == nD3DModeIndex ) 
							{
								DisplayMode.Width  = __aWindowResArray[i].nWidth;
								DisplayMode.Height = __aWindowResArray[i].nHeight;
								DisplayMode.Format = OrigDisplayMode.Format;
								DisplayMode.RefreshRate = 1;	// Note: we cheat and use RefreshRate as bWindowed
								break;
							}
						}
					}
				}

				if( !_FindColorFmtInfo( DisplayMode.Format ) ) 
				{
					// This format isn't recognized by this module's tables. Skip it...
					continue;
				}

				// Filter out refresh rates...
				for( m=0; m<nModeCount; m++ ) 
				{
					if( (aMode[m].Width  == DisplayMode.Width ) &&
						(aMode[m].Height == DisplayMode.Height) &&
						(aMode[m].Format == DisplayMode.Format) &&
						(aMode[m].RefreshRate == DisplayMode.RefreshRate) ) 
					{	// Note: RefreshRate is really bWindowed
						// Mode already exists...
						break;
					}
				}
				if( m == nModeCount ) 
				{
					// We found a new mode. Add it to our list of modes...
					aMode[nModeCount++] = DisplayMode;

					// Check if the mode's format already exists in our list of formats...
					for( f=0; f<nCFormatCount; f++ ) 
					{
						if( aCFormat[f] == DisplayMode.Format ) 
						{
							break;
						}
					}
					if( f == nCFormatCount ) 
					{
						if( nCFormatCount >= _MAX_D3D_FORMATS_PER_ADAPTER ) 
						{
							// Too many formats. Can't add this mode...
							nModeCount--;
							continue;
						}

						// We found a new format. Skip if it can't be used as a render target...
						if( FAILED( FDX8_pD3D->CheckDeviceType( nD3DAdapterIndex, _EnumD3DDevType, DisplayMode.Format, DisplayMode.Format, !bScanningFullscreen ) ) ) 
						{
							// Format doesn't check out. Skip it...
							nModeCount--;
							continue;
						}

						// Now find the list of depth/stencil buffer formats for this color format...
						anDSFormatCount[nCFormatCount] = 0;

						for( d=0; __aDSFormat[d]!=D3DFMT_UNKNOWN; d++ ) 
						{
							if( _CheckDepthStencilFormat( nD3DAdapterIndex, _EnumD3DDevType, DisplayMode.Format, __aDSFormat[d] ) ) 
							{
								// Format supported...
								aDSFormat[nCFormatCount][ anDSFormatCount[nCFormatCount]++ ] = __aDSFormat[d];
							} 
							else if( __aDSFormat[d] == D3DFMT_D16 ) 
							{
								// D3DFMT_D16 format is not supported. Try D3DFMT_D16_LOCKABLE instead...
								
								if( _CheckDepthStencilFormat( nD3DAdapterIndex, _EnumD3DDevType, DisplayMode.Format, D3DFMT_D16_LOCKABLE ) ) 
								{
									// D3DFMT_D16_LOCKABLE format supported...
									aDSFormat[nCFormatCount][ anDSFormatCount[nCFormatCount]++ ] = D3DFMT_D16_LOCKABLE;
								}
							}
						}

						if( anDSFormatCount[nCFormatCount] == 0 ) 
						{
							// No depth/stencil formats work with this color format.
							// Skip this color format...
							nModeCount--;
							continue;
						}

						// Add format to our list of unique formats...

						aCFormat[nCFormatCount] = DisplayMode.Format;
						nCFormatCount++;
					}
				}
			}
		}

		// Now we have a unique list of color formats, each with a list of unique depth/stencil formats!

		pAdapterInfo->nVidModeCount = 0;

		for( m=0; m<nModeCount; m++ ) 
		{
			const _D3DColorFmtInfo_t *pCFmtInfo1, *pCFmtInfo2;
			const _D3DDepthFmtInfo_t *pDFmtInfo1, *pDFmtInfo2;
			_VidModeInfo_t VidModeInfo, *pVidModeInfo;

			if( pAdapterInfo->nVidModeCount >= _MAX_VIDMODES ) 
			{
				// No more space to create more VidModes...
				break;
			}

			pCFmtInfo1 = _FindColorFmtInfo( aMode[m].Format );

			VidModeInfo.VidMode.nFlags = aMode[m].RefreshRate ? FVID_MODEFLAG_WINDOWED : 0;	// Remember, we're using RefreshRate as bWindowed
			VidModeInfo.VidMode.nColorBits = pCFmtInfo1->nColorBits;
			VidModeInfo.VidMode.nPixelsAcross = aMode[m].Width;
			VidModeInfo.VidMode.nPixelsDown = aMode[m].Height;
			VidModeInfo.DisplayMode = aMode[m];

			for( f=0; f<nCFormatCount; f++ ) 
			{
				if( aCFormat[f] == aMode[m].Format ) 
				{
					break;
				}
			}
			FASSERT( f<nCFormatCount );

			for( d=0; d<anDSFormatCount[f]; d++ ) 
			{
				if( pAdapterInfo->nVidModeCount >= _MAX_VIDMODES ) 
				{
					// No more space to create more VidModes...
					break;
				}

				pDFmtInfo1 = _FindDepthFmtInfo( aDSFormat[f][d] );

				VidModeInfo.VidMode.nDepthBits = pDFmtInfo1->nDepthBits;
				VidModeInfo.VidMode.nStencilBits = pDFmtInfo1->nStencilBits;
				VidModeInfo.nDepthFormat = aDSFormat[f][d];

				// Find out if we already have an identical VidMode...
				for( i=0; i<pAdapterInfo->nVidModeCount; i++ ) 
				{
					pVidModeInfo = &pVidModeArray[i];

					if( VidModeInfo.VidMode.nFlags		  == pVidModeInfo->VidMode.nFlags &&
						VidModeInfo.VidMode.nColorBits    == pVidModeInfo->VidMode.nColorBits &&
						VidModeInfo.VidMode.nPixelsAcross == pVidModeInfo->VidMode.nPixelsAcross &&
						VidModeInfo.VidMode.nPixelsDown   == pVidModeInfo->VidMode.nPixelsDown &&
						VidModeInfo.VidMode.nDepthBits    == pVidModeInfo->VidMode.nDepthBits &&
						VidModeInfo.VidMode.nStencilBits  == pVidModeInfo->VidMode.nStencilBits ) 
					{
						// Redundant VidMode. Is the new one more preferable?

						pCFmtInfo2 = _FindColorFmtInfo( pVidModeInfo->DisplayMode.Format );
						if( pCFmtInfo1->nPref > pCFmtInfo2->nPref ) 
						{
							// New VidMode is more preferable, so overwrite old with new...
							*pVidModeInfo = VidModeInfo;
						} 
						else if( pCFmtInfo1->nPref == pCFmtInfo2->nPref ) 
						{
							// Equal preferences. Check depth/stencil preference...

							pDFmtInfo2 = _FindDepthFmtInfo( pVidModeInfo->nDepthFormat );
							if( pDFmtInfo1->nPref > pDFmtInfo2->nPref ) 
							{
								// New VidMode is more preferable, so overwrite old with new...
								*pVidModeInfo = VidModeInfo;
							}
						}

						break;
					}
				}
				if( i == pAdapterInfo->nVidModeCount ) 
				{
					// Unique VidMode. Add it...
					pVidModeArray[ pAdapterInfo->nVidModeCount++ ] = VidModeInfo;
				}
			}
		}

		if( pAdapterInfo->nVidModeCount > 0 ) 
		{
			// There is at least one video mode for this adapter...

			pAdapterInfo->pVidModeArray = (_VidModeInfo_t *)fang_Malloc( pAdapterInfo->nVidModeCount * sizeof(_VidModeInfo_t), 0 );
			if( pAdapterInfo->pVidModeArray == NULL ) 
			{
				// Not enough memory...
				continue;
			}

			fang_MemCopy( pAdapterInfo->pVidModeArray, pVidModeArray, pAdapterInfo->nVidModeCount * sizeof(_VidModeInfo_t) );
		} 
		else
		{
			// No video modes for this adapter...
			continue;
		}

		qsort( pAdapterInfo->pVidModeArray, pAdapterInfo->nVidModeCount, sizeof(_VidModeInfo_t), _SortVidModesCallback );

		// Add adapter...
		_nAdapterArrayCount++;
	}

	fang_Free( pVidModeArray );
}


//
//
//
static int _SortVidModesCallback( const void *pArg1, const void *pArg2 ) 
{
	_VidModeInfo_t *pVidMode1 = (_VidModeInfo_t *)pArg1;
	_VidModeInfo_t *pVidMode2 = (_VidModeInfo_t *)pArg2;

	if( (pVidMode1->VidMode.nFlags & FVID_MODEFLAG_WINDOWED) > (pVidMode2->VidMode.nFlags & FVID_MODEFLAG_WINDOWED) ) return -1;
	if( (pVidMode1->VidMode.nFlags & FVID_MODEFLAG_WINDOWED) < (pVidMode2->VidMode.nFlags & FVID_MODEFLAG_WINDOWED) ) return +1;

	if( pVidMode1->VidMode.nColorBits < pVidMode2->VidMode.nColorBits ) return -1;
	if( pVidMode1->VidMode.nColorBits > pVidMode2->VidMode.nColorBits ) return +1;

	if( pVidMode1->VidMode.nDepthBits < pVidMode2->VidMode.nDepthBits ) return -1;
	if( pVidMode1->VidMode.nDepthBits > pVidMode2->VidMode.nDepthBits ) return +1;

	if( pVidMode1->VidMode.nStencilBits < pVidMode2->VidMode.nStencilBits ) return -1;
	if( pVidMode1->VidMode.nStencilBits > pVidMode2->VidMode.nStencilBits ) return +1;

	if( pVidMode1->VidMode.nPixelsAcross < pVidMode2->VidMode.nPixelsAcross ) return -1;
	if( pVidMode1->VidMode.nPixelsAcross > pVidMode2->VidMode.nPixelsAcross ) return +1;

	if( pVidMode1->VidMode.nPixelsDown < pVidMode2->VidMode.nPixelsDown ) return -1;
	if( pVidMode1->VidMode.nPixelsDown > pVidMode2->VidMode.nPixelsDown ) return +1;

	return 0;
}


//
//
//
static const _D3DColorFmtInfo_t *_FindColorFmtInfo( D3DFORMAT nFormat ) 
{
	for( const _D3DColorFmtInfo_t *pColorInfo=_aD3DColorFmtInfo; pColorInfo->nFormat!=D3DFMT_UNKNOWN; pColorInfo++ ) 
	{
		if( pColorInfo->nFormat == nFormat ) 
		{
			return pColorInfo;
		}
	}

	return NULL;
}


//
//
//
static const _D3DDepthFmtInfo_t *_FindDepthFmtInfo( D3DFORMAT nFormat ) 
{
	for( const _D3DDepthFmtInfo_t *pDepthInfo=_aD3DDepthFmtInfo; pDepthInfo->nFormat!=D3DFMT_UNKNOWN; pDepthInfo++ ) 
	{
		if( pDepthInfo->nFormat == nFormat ) 
		{
			return pDepthInfo;
		}
	}

	return NULL;
}


//
//
//
static BOOL _CheckDepthStencilFormat( UINT nD3DAdapterIndex, D3DDEVTYPE DevType, D3DFORMAT nColorFormat, D3DFORMAT nDepthStencilFormat ) 
{
	// See if this depth/stencil format is supported at all by the device...
	if( FAILED( FDX8_pD3D->CheckDeviceFormat( nD3DAdapterIndex, DevType, nColorFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, nDepthStencilFormat ) ) ) 
	{
		// Not supported...
		return FALSE;
	}

	// See if this depth/stencil format is compatible with our render target format...
	if( FAILED( FDX8_pD3D->CheckDepthStencilMatch( nD3DAdapterIndex, DevType, nColorFormat, nColorFormat, nDepthStencilFormat ) ) ) 
	{
		// Not supported...
		return FALSE;
	}

	// Supported...

	return TRUE;
}


//
//
//
static D3DMULTISAMPLE_TYPE _GetMultiSampleType( const _AdapterInfo_t *pAdapterInfo, const _VidModeInfo_t *pVidModeInfo, f32 *pfUnitFSAA ) 
{
	u32 i, nTableEntryCount, nTableIndex;

	static struct 
	{
		D3DMULTISAMPLE_TYPE nD3DMultiSampleType;
		BOOL8 bSupported;
		BOOL8 bTableEnd;
	} __aMultiSampleTable[] = {
#if FANG_PLATFORM_WIN
		D3DMULTISAMPLE_NONE,		FALSE,	FALSE,	// D3DMULTISAMPLE_NONE must be the first entry,
		D3DMULTISAMPLE_2_SAMPLES,	FALSE,	FALSE,	//   and these values from range from low to high...
		D3DMULTISAMPLE_3_SAMPLES,	FALSE,	FALSE,
		D3DMULTISAMPLE_4_SAMPLES,	FALSE,	FALSE,
		D3DMULTISAMPLE_5_SAMPLES,	FALSE,	FALSE,
		D3DMULTISAMPLE_6_SAMPLES,	FALSE,	FALSE,
		D3DMULTISAMPLE_7_SAMPLES,	FALSE,	FALSE,
		D3DMULTISAMPLE_8_SAMPLES,	FALSE,	FALSE,
		D3DMULTISAMPLE_9_SAMPLES,	FALSE,	FALSE,
		D3DMULTISAMPLE_10_SAMPLES,	FALSE,	FALSE,
		D3DMULTISAMPLE_11_SAMPLES,	FALSE,	FALSE,
		D3DMULTISAMPLE_12_SAMPLES,	FALSE,	FALSE,
		D3DMULTISAMPLE_13_SAMPLES,	FALSE,	FALSE,
		D3DMULTISAMPLE_14_SAMPLES,	FALSE,	FALSE,
		D3DMULTISAMPLE_15_SAMPLES,	FALSE,	FALSE,
		D3DMULTISAMPLE_16_SAMPLES,	FALSE,	FALSE,

		D3DMULTISAMPLE_NONE,		FALSE,	TRUE
#else
		D3DMULTISAMPLE_NONE,		FALSE,	FALSE,	// D3DMULTISAMPLE_NONE must be the first entry,
		D3DMULTISAMPLE_2_SAMPLES,	FALSE,	FALSE,	//   and these values from range from low to high...
		D3DMULTISAMPLE_4_SAMPLES,	FALSE,	FALSE,
		D3DMULTISAMPLE_9_SAMPLES,	FALSE,	FALSE,

		D3DMULTISAMPLE_NONE,		FALSE,	TRUE
#endif
	};

	// Count the entries in the table
	nTableEntryCount = 0;
	for( i=0; !__aMultiSampleTable[i].bTableEnd; i++ ) nTableEntryCount++;
	FASSERT( nTableEntryCount );

	__aMultiSampleTable[0].bSupported = TRUE;

	for( i=1; i<nTableEntryCount; i++ ) 
	{
		__aMultiSampleTable[i].bSupported = TRUE;

		if( FDX8_pD3D->CheckDeviceMultiSampleType(
				pAdapterInfo->nD3DAdapterIndex,
				_D3DDevType,
				pVidModeInfo->DisplayMode.Format,
				!!(pVidModeInfo->VidMode.nFlags & FVID_MODEFLAG_WINDOWED),
				__aMultiSampleTable[i].nD3DMultiSampleType
			) != D3D_OK ) 
		{
			// Multisample type is not supported in color buffer...
			__aMultiSampleTable[i].bSupported = FALSE;
			continue;
		}

		if( FDX8_pD3D->CheckDeviceMultiSampleType(
				pAdapterInfo->nD3DAdapterIndex,
				_D3DDevType,
				pVidModeInfo->nDepthFormat,
				!!(pVidModeInfo->VidMode.nFlags & FVID_MODEFLAG_WINDOWED),
				__aMultiSampleTable[i].nD3DMultiSampleType
			) != D3D_OK ) 
		{
			// Multisample type is supported in depth/stencil buffer...
			__aMultiSampleTable[i].bSupported = FALSE;
		}
	}

	nTableIndex = (u32)( *pfUnitFSAA * (float)nTableEntryCount );
	if( nTableIndex == nTableEntryCount ) 
	{
		nTableIndex--;
	}

	for( ; !__aMultiSampleTable[nTableIndex].bSupported; nTableIndex-- );

	*pfUnitFSAA = (float)nTableIndex / (float)(nTableEntryCount-1);

	return __aMultiSampleTable[nTableIndex].nD3DMultiSampleType;
}


//
//
//
static DWORD _ComputePresentationInterval( u32 *pnSwapInterval, const D3DCAPS8 *pCaps ) 
{
	FASSERT( *pnSwapInterval>=0 && *pnSwapInterval<=2 );

#if _OVERRIDE_SWAP_INTERVAL
//	return D3DPRESENT_INTERVAL_TWO;
//	return D3DPRESENT_INTERVAL_ONE;
	return D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE;
//	return D3DPRESENT_INTERVAL_IMMEDIATE;
#endif

	switch( *pnSwapInterval ) 
	{
	case 0:
		// Immedate...
		if( pCaps->PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE ) 
		{
			*pnSwapInterval = 0;
			return D3DPRESENT_INTERVAL_IMMEDIATE;
		}

	case 1:
		// Next vblank...
		#if FANG_PLATFORM_XB
			if( pCaps->PresentationIntervals & D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE  ) 
			{
				*pnSwapInterval = 1;
				return D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE;
			}
		#endif

		if( pCaps->PresentationIntervals & D3DPRESENT_INTERVAL_ONE ) 
		{
			*pnSwapInterval = 1;
			return D3DPRESENT_INTERVAL_ONE;
		}
		if( pCaps->PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE ) 
		{
			*pnSwapInterval = 0;
			return D3DPRESENT_INTERVAL_IMMEDIATE;
		}

	case 2:
		// Next vblank...
		if( pCaps->PresentationIntervals & D3DPRESENT_INTERVAL_TWO ) 
		{
			*pnSwapInterval = 2;
			return D3DPRESENT_INTERVAL_TWO;
		}
		if( pCaps->PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE ) 
		{
			*pnSwapInterval = 0;
			return D3DPRESENT_INTERVAL_IMMEDIATE;
		}
		if( pCaps->PresentationIntervals & D3DPRESENT_INTERVAL_THREE ) 
		{
			*pnSwapInterval = 3;
			return D3DPRESENT_INTERVAL_THREE;
		}
		if( pCaps->PresentationIntervals & D3DPRESENT_INTERVAL_FOUR ) 
		{
			*pnSwapInterval = 4;
			return D3DPRESENT_INTERVAL_FOUR;
		}
	}

	// Uhhhhhhhhhhhh.... not quite sure what to do here.
	// Guess I'll just return DEFAULT, even though it's not really
	// a valid flag. We'll just let CreateDevice handle the
	// failure...

	return D3DPRESENT_INTERVAL_DEFAULT;
}

// Returns TRUE if successful, or FALSE if a catastrophic error
// occurred. If a catastrophic error is reported, the game has
// no choice but to shut down without any further graphics
// processing.
static BOOL _SwapBuffers( void ) 
{
#if FANG_PLATFORM_WIN
	if( FDX8_pDev->Present( NULL, NULL, NULL, NULL ) == D3DERR_DEVICELOST ) 
	{
		// Device lost...

		_bDeviceLost = TRUE;

		if( FDX8_pDev->TestCooperativeLevel() == D3DERR_DEVICENOTRESET ) 
		{
			// DX8 says it's ok to reset the device now...

			// Notify modules that we're about to reset the device...
			_CallWindowCallbackFunctions( FDX8VID_EVENT_PRE_RESET );

			// Destroy our depth/stencil and back buffers...
			FDX8_SAFE_RELEASE( FDX8_pSurfDepthStencil );
			FDX8_SAFE_RELEASE( FDX8_pSurfBack );

			if( FAILED( FDX8_pDev->Reset( &_PresentationParms ) ) ) 
			{
				// Catastrophic failure. We have no choice but to
				// shut down the video window and report back a failure
				// to the game.
				//
				// At this point, the only D3D operations that the engine
				// can use is ->Release(). We set fdx8vid_bResetting
				// to indicate this.

				_bResetFailed = TRUE;
				_bDeviceLost = FALSE;

				fdx8vid_bResetting = TRUE;
				fvid_DestroyWindow();
				fdx8vid_bResetting = FALSE;

				return FALSE;
			}

			// Recreate our depth/stencil and back buffers...
			FDX8_pDev->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &FDX8_pSurfBack );
			FDX8_pDev->GetDepthStencilSurface( &FDX8_pSurfDepthStencil );

			if( _hCursor ) 
			{
				fdx8vid_SetCursor( _hCursor );
			}

			// Notify modules that we're done resetting...
			fdx8_InitD3DState();
			_CallWindowCallbackFunctions( FDX8VID_EVENT_POST_RESET );

			_bDeviceLost = FALSE;
		}
	}
#else
/*
	while ( TRUE )
	{
		if ( _bBufferVSyncHit[_nXBPresentCalls & 0x01] )
		{
			_bBufferVSyncHit[_nXBPresentCalls & 0x01] = FALSE;
			break;
		}
	}
	_nXBPresentCalls++;
*/
	FDX8_pDev->Present( NULL, NULL, NULL, NULL );
#endif

	FVid_nSwapCounter++;

	return TRUE;
}


#if FANG_PLATFORM_XB
//
//
//
void _XboxSwapCallback( D3DSWAPDATA *pData )
{
	_bBufferSwapReady[(pData->Swap & 0x01)] = TRUE;
	_nXBSwapCallbackHit++;
}


//
//
//
void _XboxVBlankCallback( D3DVBLANKDATA *pData )
{
	_bBufferVSyncHit[(pData->Swap & 0x01)] = TRUE;
}
#endif // FANG_PLATFORM_XB


//
//
//
static BOOL _CreateWindow( void ) 
{
	DWORD nClearFlags;

//	D3DPERF_SetShowFrameRateInterval( 5000 );
//	D3DPERF_GetStatistics()->m_dwDumpFPSInfoMask |= D3DPERF_DUMP_FPS_PERFPROFILE;

	_bMinimized = FALSE;
	_bUserClickedClose = FALSE;
	_bReady = FALSE;
	_bResetFailed = FALSE;
	_bDeviceLost = FALSE;
	_bHaveFocus = FALSE;
	_bLeftMouseButton = FALSE;
	_bRightMouseButton = FALSE;
	_hCursor = 0;
	_hWnd = 0;

	if( FAILED( FDX8_pD3D->GetAdapterIdentifier( _CurAdapterInfo.nD3DAdapterIndex, 0, &_AdapterID ) ) ) 
	{
		// Could not get adapter ID. Skip this adapter...
		return FALSE;
	}

#if FANG_PLATFORM_WIN
	if( FDX8_pD3D->GetAdapterDisplayMode( _CurAdapterInfo.nD3DAdapterIndex, &_OrigDisplayMode ) != D3D_OK ) 
	{
		// Trouble obtaining the current display mode. Skip this adapter...
		return FALSE;
	}
#endif

	if( FAILED( FDX8_pD3D->GetDeviceCaps( _CurAdapterInfo.nD3DAdapterIndex, _D3DDevType, &_DeviceCaps ) ) ) 
	{
		// Could not retrieve device caps. Skip this adapter...
		return FALSE;
	}

#if FANG_PLATFORM_WIN
	DWORD nWindowStyle;
	RECT Rect;

	_hWnd = FVid_Win.hWnd;

	if( _hWnd == NULL ) 
	{
		// We must create the rendering window...

		_bWeCreatedTheWindow = TRUE;

		_RegisterWindowsClass();

		if( FVid_Mode.nFlags & FVID_MODEFLAG_WINDOWED ) 
		{
			nWindowStyle = WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;
#if FANG_WINGC
			if( FDX8Vid_bPortBorderless ) {
				nWindowStyle = WS_POPUP | WS_VISIBLE;
			}
#endif
		} 
		else 
		{
			nWindowStyle = WS_POPUP | WS_VISIBLE;
		}

		SetRect( &Rect, 0, 0, FVid_Mode.nPixelsAcross, FVid_Mode.nPixelsDown );
		AdjustWindowRect( &Rect, nWindowStyle, FALSE );
#if FANG_WINGC
		// Borderless: a captionless window over the whole primary display, at its origin.
		int nWindowX = CW_USEDEFAULT, nWindowY = CW_USEDEFAULT;
		if( (FVid_Mode.nFlags & FVID_MODEFLAG_WINDOWED) && FDX8Vid_bPortBorderless ) {
			nWindowX = 0;
			nWindowY = 0;
		} else if( FVid_Mode.nFlags & FVID_MODEFLAG_WINDOWED ) {
			// Center a normal window on the desktop's work area.
			RECT Work;
			if( SystemParametersInfo( SPI_GETWORKAREA, 0, &Work, 0 ) ) {
				nWindowX = Work.left + FMATH_MAX( 0, ((Work.right - Work.left) - (Rect.right - Rect.left)) / 2 );
				nWindowY = Work.top + FMATH_MAX( 0, ((Work.bottom - Work.top) - (Rect.bottom - Rect.top)) / 2 );
			}
		}
#else
		const int nWindowX = CW_USEDEFAULT, nWindowY = CW_USEDEFAULT;
#endif

#ifdef _UNICODE
		static TCHAR wszTitle[MAX_PATH];
		MultiByteToWideChar( CP_THREAD_ACP, MB_PRECOMPOSED, FVid_Win.szWindowTitle, strlen( FVid_Win.szWindowTitle ), wszTitle, sizeof( wszTitle ) );
		_hWnd = CreateWindow( _WINDOW_CLASS_NAME, wszTitle, nWindowStyle, nWindowX, nWindowY,
							  (Rect.right-Rect.left), (Rect.bottom-Rect.top), 0L, NULL, FVid_Win.hInstance, 0L );
#else
		_hWnd = CreateWindow( _WINDOW_CLASS_NAME, FVid_Win.szWindowTitle, nWindowStyle, nWindowX, nWindowY,
							  (Rect.right-Rect.left), (Rect.bottom-Rect.top), 0L, NULL, FVid_Win.hInstance, 0L );
#endif
	} 
	else 
	{
		// The caller is providing the rendering window for us...

		_bWeCreatedTheWindow = FALSE;

		if( FVid_Mode.nFlags & FVID_MODEFLAG_WINDOWED ) 
		{
			nWindowStyle = GetWindowLong( _hWnd, GWL_STYLE );
			SetRect( &Rect, 0, 0, FVid_Mode.nPixelsAcross, FVid_Mode.nPixelsDown );
			AdjustWindowRect( &Rect, nWindowStyle, FALSE );

			SetWindowPos(
				_hWnd,
				HWND_NOTOPMOST,
				0,
				0,
				(Rect.right - Rect.left),
				(Rect.bottom - Rect.top),
				SWP_SHOWWINDOW
			);
		}
	}

	// Removed by Justin - 1/15/02 - I can't find any reason why this should be called.
//	_SetClientSize( FVid_Mode.nPixelsAcross, FVid_Mode.nPixelsDown );
	_nWindowStyle = GetWindowLong( _hWnd, GWL_STYLE );
	GetWindowRect( _hWnd, &_WindowRect );
	GetClientRect( _hWnd, &_ClientRect );
#endif

	FDX8Vid_nBackBufferMultisampleType = _GetMultiSampleType( &_CurAdapterInfo, &_CurVidModeInfo, &FVid_Win.fUnitFSAA );

	if ( fsysinfo_GetCapabilities() & FSYSINFO_VIDEO_PAL_60HZ )
	{
		// This will handle PAL-60
		_nRefreshRate = 60;
	}
	else
	{
		_nRefreshRate = D3DPRESENT_RATE_DEFAULT;
	}

	if( FVid_Mode.nFlags & FVID_MODEFLAG_WINDOWED ) 
	{
		_nPresentationInterval = 0;
	} 
	else 
	{
		_nPresentationInterval = _ComputePresentationInterval( &FVid_Win.nSwapInterval, &_DeviceCaps );
	}

#if FANG_PLATFORM_WIN
	if( _DeviceCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) 
	{
		//_nBehaviorFlags = D3DCREATE_MIXED_VERTEXPROCESSING;
		_nBehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	} 
	else 
	{
		_nBehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
#else
	_nBehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
#endif

	FANG_ZEROSTRUCT( _PresentationParms );

	if( FVid_Mode.nFlags & FVID_MODEFLAG_WINDOWED ) 
	{
		_PresentationParms.BackBufferWidth = _ClientRect.right - _ClientRect.left;
		_PresentationParms.BackBufferHeight = _ClientRect.bottom - _ClientRect.top;
	} 
	else 
	{
		_PresentationParms.BackBufferWidth = FVid_Mode.nPixelsAcross;
		_PresentationParms.BackBufferHeight = FVid_Mode.nPixelsDown;
	}

	_PresentationParms.BackBufferFormat = _CurVidModeInfo.DisplayMode.Format;
	_PresentationParms.BackBufferCount = 1;
	_PresentationParms.MultiSampleType = FDX8Vid_nBackBufferMultisampleType;
	_PresentationParms.SwapEffect = D3DSWAPEFFECT_DISCARD;
	_PresentationParms.hDeviceWindow = _hWnd;
	_PresentationParms.Windowed = !!(FVid_Mode.nFlags & FVID_MODEFLAG_WINDOWED);
	_PresentationParms.EnableAutoDepthStencil = TRUE;
	_PresentationParms.AutoDepthStencilFormat = _CurVidModeInfo.nDepthFormat;
#if FANG_PLATFORM_WIN
	_PresentationParms.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
#else
	_PresentationParms.Flags = 0;
#endif
	_PresentationParms.FullScreen_RefreshRateInHz = _nRefreshRate;
	_PresentationParms.FullScreen_PresentationInterval = _nPresentationInterval;

#if FANG_PLATFORM_XB
	FDX8_pD3D->SetPushBufferSize( (DWORD)(1.5f * 1024.f * 1024.f), 64 * 1024 );
#endif

	if( FAILED( FDX8_pD3D->CreateDevice( _CurAdapterInfo.nD3DAdapterIndex, _D3DDevType, _hWnd, _nBehaviorFlags, &_PresentationParms, &FDX8_pDev ) ) ) {
#if FANG_PLATFORM_WIN
		DestroyWindow( _hWnd );
		return FALSE;
	}
	else
	{
		_D3DSubclassedWndProc = (WNDPROC)SetWindowLong(_hWnd, GWL_WNDPROC,(LONG) _UnSubclassD3DsWndProc);
#endif
	}


	FDX8_pDev->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &FDX8_pSurfBack );
	FDX8_pDev->GetDepthStencilSurface( &FDX8_pSurfDepthStencil );
	FDX8_pDev->GetDeviceCaps( &FDX8_Caps );
	FDX8_pSurfBack->GetDesc( &FDX8_BackDesc );
	FDX8_pSurfDepthStencil->GetDesc( &FDX8_DepthStencilDesc );

	_bReady = TRUE;
	FVid_bOnline = TRUE;

	_bMinimized = FALSE;
	_bBeginEndCalled = FALSE;
	FVid_bBegin = FALSE;
	FVid_nFrameCounter = 1;

	_nCursorX = FVid_Mode.nPixelsAcross >> 1;
	_nCursorY = FVid_Mode.nPixelsDown >> 1;
	_bCursorOn = !!(FVid_Mode.nFlags & FVID_MODEFLAG_WINDOWED);

#if FANG_PLATFORM_WIN
	// disable the screensaver from kicking on
	SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, FALSE, 0, 0 );

#if defined(MA_PC_INPUT)
	// Startup may still display Windows' app-starting cursor. Do not store it
	// as this window's permanent cursor when gameplay releases the mouse.
	fdx8vid_SetCursor( LoadCursor( NULL, IDC_ARROW ) );
#else
	fdx8vid_SetCursor( GetCursor() );
#endif
#endif

	// Clear back color, depth, and stencil buffers...
	nClearFlags = D3DCLEAR_TARGET;
	if( FVid_Mode.nStencilBits ) 
	{
		nClearFlags |= D3DCLEAR_STENCIL;
	}
	if( FVid_Mode.nDepthBits ) 
	{
		nClearFlags |= D3DCLEAR_ZBUFFER;
	}
	FDX8_pDev->Clear( 0, NULL, nClearFlags, 0, FVid_Mode.nDepthBits ? 1.0f : 0.0f, 0 );

#if FANG_PLATFORM_XB
//	D3DPERF_SetShowFrameRateInterval( 960 );
	_bBufferSwapReady[0] = _bBufferSwapReady[1] = TRUE;
	_bBufferVSyncHit[0] = _bBufferVSyncHit[1] = TRUE;

	_nXBSwapCallbackHit = _nXBPresentCalls = 0;
	FDX8_pDev->SetSwapCallback( _XboxSwapCallback );
	FDX8_pDev->SetVerticalBlankCallback( _XboxVBlankCallback );
#endif

	FDX8_pDev->GetGammaRamp( &_GammaRamp );

	u32 i;
	f32 fValue;
	for ( i = 0; i < 128; i++ )
	{
		fValue = 127.f * ((f32)i/127.f * (f32)i/127.f);
		_GammaRamp.red[i] = (u8)fValue;
		_GammaRamp.green[i] = (u8)fValue;
		_GammaRamp.blue[i] = (u8)fValue;

		_GammaRamp.red[255-i] = 255 - (u8)fValue;
		_GammaRamp.green[255-i] = 255 - (u8)fValue;
		_GammaRamp.blue[255-i] = 255 - (u8)fValue;
	}

//	FDX8_pDev->SetGammaRamp( 0, &_GammaRamp );

	return TRUE;
}


//
//
//
static void _DestroyWindow( void ) 
{
#if FANG_PLATFORM_WIN
	// restore the WndProc:
	if (_D3DSubclassedWndProc)
		SetWindowLong(_hWnd, GWL_WNDPROC,(LONG) _D3DSubclassedWndProc);
#endif 

	_bReady = FALSE;
	FVid_bOnline = FALSE;

	FDX8_SAFE_RELEASE( FDX8_pSurfDepthStencil );
	FDX8_SAFE_RELEASE( FDX8_pSurfBack );
	u32 nCnt=0;
	if (FDX8_pDev)
	{
		do
		{
			nCnt = FDX8_pDev->Release();
		} while (nCnt);
	}
	if (FDX8_pD3D)
	{
		/*do
		{
			nCnt = FDX8_pD3D->Release();
		} while (nCnt);*/
		// NKM - I added the FDX8_SAFE_RELEASE() since in Release mode the nCnt was always 1
		FDX8_SAFE_RELEASE( FDX8_pD3D );
	}
	//FDX8_SAFE_RELEASE( FDX8_pDev );
    //FDX8_SAFE_RELEASE( FDX8_pD3D );

#if FANG_PLATFORM_WIN
	// enable the screensaver 
	SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, TRUE, 0, 0 );

	if( _bWeCreatedTheWindow ) 
	{
		DestroyWindow( _hWnd );
	}
#endif

	_ClearVideoData();
}


#if FANG_PLATFORM_WIN
//
//
//
static void _SetClientSize( int nClientWidth, int nClientHeight ) 
{
	// Steve's old code.
/*	RECT WindowRect, ClientRect;

	GetWindowRect( _hWnd, &WindowRect );
	GetClientRect( _hWnd, &ClientRect );

	MoveWindow(
		_hWnd,
		WindowRect.left,
		WindowRect.top,
		WindowRect.right - WindowRect.left + (nClientWidth - ClientRect.right),
		WindowRect.bottom - WindowRect.top + (nClientHeight - ClientRect.bottom),
		TRUE
	);*/

	////////////////////////////////////////////

	// Justin's code.  Neither of these are quite right but it doesn't matter, as it doesn't
	//   seem there is any good reason to call this function.
	RECT DestWndRect;
	POINT ClientPt;

	GetWindowRect( _hWnd, &DestWndRect );
	ClientPt.x = DestWndRect.left;
	ClientPt.y = DestWndRect.top;
	ScreenToClient( GetParent( _hWnd ), &ClientPt );

	MoveWindow(
		_hWnd,
		ClientPt.x,
		ClientPt.y,
		nClientWidth,
		nClientHeight,
		TRUE
	);
}


//
//
//
static void _RegisterWindowsClass( void ) 
{
	WNDCLASS WinClass;

	// Register our new class...

	FANG_ZEROSTRUCT( WinClass );

	WinClass.style = 0;
	WinClass.lpfnWndProc = _WndProc;
	WinClass.cbClsExtra = 0;
	WinClass.cbWndExtra = 0;
	WinClass.hInstance = FVid_Win.hInstance;
	WinClass.hIcon = (FVid_Win.nIconIDI == NULL) ? LoadIcon( NULL, IDI_APPLICATION ) : LoadIcon( FVid_Win.hInstance, MAKEINTRESOURCE(FVid_Win.nIconIDI) );
	WinClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	WinClass.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	WinClass.lpszMenuName = NULL;
	WinClass.lpszClassName = _WINDOW_CLASS_NAME;

	RegisterClass( &WinClass );
}
#endif


//
//
//
static int _FindWindowCallbackFunction( FDX8VidWindowCallback_t *pFcnWindowCallback ) 
{
	int i;

	for( i=0; i<_nWindowCallbackCount; i++ ) 
	{
		if( _ppFcnWindowCallbackTable[i] == pFcnWindowCallback ) 
		{
			// Found it...
			return i;
		}
	}

	// Not found...

	return -1;
}


//
//
//
static BOOL _CallWindowCallbackFunctions( FDX8VidEvent_e nEvent ) 
{
	BOOL bCallInReverseOrder;
	int i;

	if( nEvent == FDX8VID_EVENT_WINDOW_CREATED ) 
	{
		_MasterResFrame = fres_GetFrame();
	} 
	else if( nEvent == FDX8VID_EVENT_WINDOW_DESTROYED ) 
	{
		fres_ReleaseFrame( _MasterResFrame );
	}

	bCallInReverseOrder = (nEvent==FDX8VID_EVENT_WINDOW_DESTROYED || nEvent==FDX8VID_EVENT_PRE_RESET );

	if( !bCallInReverseOrder ) 
	{
		// Callback in forward order...

		for( i=0; i<_nWindowCallbackCount; i++ ) 
		{
			if( !(_ppFcnWindowCallbackTable[i])( nEvent ) ) 
			{
				// Callback returned an error...

				// If that was a window-created event, call the already-visited
				// functions with the window-destroyed event. Otherwise, ignore
				// the return code...
				if( nEvent == FDX8VID_EVENT_WINDOW_CREATED ) 
				{
					fres_ReleaseFrame( _MasterResFrame );

					for( i--; i>=0; i-- ) 
					{
						(_ppFcnWindowCallbackTable[i])( FDX8VID_EVENT_WINDOW_DESTROYED );
					}

					return FALSE;
				}
			}
		}
	} 
	else 
	{
		// Callback in reverse order...

		for( i=_nWindowCallbackCount-1; i>=0; i-- ) 
		{
			(_ppFcnWindowCallbackTable[i])( nEvent );
		}
	}

	return TRUE;
}


#if FANG_PLATFORM_WIN

//
//
//
static LRESULT CALLBACK _UnSubclassD3DsWndProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam ) {
	switch (nMsg) {
	case WM_ACTIVATEAPP:
		if (wParam == 0) {	// by snooping and denying ActivateApp(FALSE), I allow fullscreen debugging with multimon 
			return 0;		// reference: http://discuss.microsoft.com/SCRIPTS/WA-MSD.EXE?A2=ind0305c&L=directxdev&D=1&P=10583
		}
	default:
		return	CallWindowProc(_D3DSubclassedWndProc,hWnd, nMsg, wParam, lParam );
	}
}

static LRESULT CALLBACK _WndProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam ) 
{
#if defined(MA_PC_INPUT)
	if( pcinput_WindowMessage( nMsg, wParam, lParam ) ) return TRUE;
#endif
	switch( nMsg ) 
	{
	case WM_PAINT:
		// Handle paint messages when the app is not ready...
		if( FDX8_pDev && !_bReady && !_bResetFailed ) 
		{
			if( FVid_Mode.nFlags & FVID_MODEFLAG_WINDOWED ) 
			{
				FDX8_pDev->Present( NULL, NULL, NULL, NULL );
			}
		}
		break;

	case WM_SETFOCUS:
		_bHaveFocus = TRUE;
		break;

	case WM_KILLFOCUS:
		_bHaveFocus = FALSE;
		break;

	case WM_NCACTIVATE:
		if( !wParam ) 
		{
			_bMinimized = TRUE;
		} 
		else 
		{
			_bMinimized = FALSE;
		}
		break;

	case WM_SIZE:
		// Handle minimize flag...
		if( wParam == SIZE_MINIMIZED ) 
		{
			_bMinimized = TRUE;
		} 
		else 
		{
			_bMinimized = FALSE;
		}
		break;

	case WM_SETCURSOR:
		// Turn off Windows cursor in fullscreen mode...
		if( _bReady ) 
		{
			if( FVid_Mode.nFlags & FVID_MODEFLAG_WINDOWED ) 
			{
				if( !_bCursorOn ) 
				{
					SetCursor( NULL );
					return TRUE;
				}
			} 
			else 
			{
				SetCursor( NULL );
				FDX8_pDev->ShowCursor( _bCursorOn );
				return TRUE;
			}
		}
		break;

	case WM_MOUSEMOVE:
		if( _bReady ) 
		{
			POINT CursorPoint;
			GetCursorPos( &CursorPoint );
			ScreenToClient( _hWnd, &CursorPoint );

			_MouseActivity( &CursorPoint, 0 );

			if( _bCursorOn ) 
			{
				FDX8_pDev->SetCursorPosition( _nCursorX, _nCursorY, 0 );
			}
		}
		break;

	case WM_MOUSEWHEEL:
		if( _bReady ) 
		{
			POINT CursorPoint;
			CursorPoint.x = LOWORD(lParam);
			CursorPoint.y = HIWORD(lParam);

			_MouseActivity( &CursorPoint, (short)HIWORD(wParam) );
		}
		break;

	case WM_LBUTTONDOWN:
		if( _bReady ) 
		{
			POINT CursorPoint;
			CursorPoint.x = LOWORD(lParam);
			CursorPoint.y = HIWORD(lParam);

			_bLeftMouseButton = TRUE;
			_MouseActivity( &CursorPoint, 0 );

			SetCapture( _hWnd );
		}
		break;

	case WM_LBUTTONUP:
		if( _bReady ) 
		{
			POINT CursorPoint;
			CursorPoint.x = LOWORD(lParam);
			CursorPoint.y = HIWORD(lParam);

			_bLeftMouseButton = FALSE;
			_MouseActivity( &CursorPoint, 0 );

			ReleaseCapture();
		}
		break;

	case WM_RBUTTONDOWN:
		if( _bReady ) 
		{
			POINT CursorPoint;
			CursorPoint.x = LOWORD(lParam);
			CursorPoint.y = HIWORD(lParam);

			_bRightMouseButton = TRUE;
			_MouseActivity( &CursorPoint, 0 );

			SetCapture( _hWnd );
		}
		break;

	case WM_RBUTTONUP:
		if( _bReady ) 
		{
			POINT CursorPoint;
			CursorPoint.x = LOWORD(lParam);
			CursorPoint.y = HIWORD(lParam);

			_bRightMouseButton = FALSE;
			_MouseActivity( &CursorPoint, 0 );

			ReleaseCapture();
		}
		break;

	case WM_NCHITTEST:
		// Prevent the user from selecting the menu in fullscreen mode...
		if( !(FVid_Mode.nFlags & FVID_MODEFLAG_WINDOWED) ) 
		{
			return HTCLIENT;
		}
		break;

	case WM_POWERBROADCAST:
		switch( wParam ) 
		{
		case PBT_APMQUERYSUSPEND:
			// Windows is requesting permission to enter power suspend mode...
			if( FVid_Win.bAllowPowerSuspend ) 
			{
				if( !FVid_Win.pFcnSuspend || FVid_Win.pFcnSuspend(TRUE) ) 
				{
					return TRUE;
				}
			}
			return FALSE;

		case PBT_APMRESUMESUSPEND:
			// Windows is notifying us that it is exiting power suspend mode...
			if( FVid_Win.bAllowPowerSuspend ) 
			{
				if( FVid_Win.pFcnSuspend ) 
				{
					FVid_Win.pFcnSuspend( FALSE );
				}
			}
			return TRUE;
		}
		break;

	case WM_SYSCOMMAND:
		// Prevent moving/sizing and power loss in fullscreen mode...
		switch( wParam ) 
		{
			/*
		case SC_SCREENSAVE:
			// if the render window has focus, never go into screen saver mode
			if( fdx8vid_HaveFocus() ) {
				return 1;
			}
			return 1;
			break;
			*/
		case SC_MOVE:
		case SC_SIZE:
		case SC_MAXIMIZE:
		case SC_KEYMENU:
		case SC_MONITORPOWER:
			if( !(FVid_Mode.nFlags & FVID_MODEFLAG_WINDOWED) ) 
			{
				return 1;
			}
			break;
		}
		break;

	case WM_CLOSE:
		// User requested to close the window...
		_bUserClickedClose = TRUE;
		floop_PauseGameloop( FALSE );
		return 0;
	}

	return DefWindowProc( hWnd, nMsg, wParam, lParam );
}


//
//
//
static void _MouseActivity( POINT *pCursorPoint, short nDeltaWheel ) 
{
	_nCursorX = pCursorPoint->x;
	_nCursorY = pCursorPoint->y;

	if( _pMouseCallback && !_bMinimized ) 
	{
		_pMouseCallback( _nCursorX, _nCursorY, (s32)nDeltaWheel, _bLeftMouseButton, _bRightMouseButton, _pMouseCallbackParm );
	}
}
#endif
