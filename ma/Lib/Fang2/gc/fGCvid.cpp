//////////////////////////////////////////////////////////////////////////////////////
// fGCvid.cpp - Fang video system module (GameCube version).
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

#include "fang.h"

#include "fGC.h"
#include "fGCvid.h"
#include "fGCtex.h"
#include "fGCdraw.h"
#include "fGCxfm.h"
#include "fGCvb.h"
#include "fGCsh.h"

#include "ftext.h"
#include "frenderer.h"
#include "fcolor.h"
#include "fmovie2.h"
#include "floop.h"
#include "fres.h"
#include "fmesh.h"
#include "fsysinfo.h"


//////////////////////////////////////////////////////////////////////////////////////
// Local Defines:
//////////////////////////////////////////////////////////////////////////////////////

#define _USE_BUFFERED_FIFO			FALSE
#define _USE_TRIPLE_BUFFER			FALSE

#define _OVERRIDE_SYNC_TO_VBLANK	(FALSE & !(FANG_PRODUCTION_BUILD))
#define	_SHOW_FRAMERATE				(FALSE & !(FANG_PRODUCTION_BUILD))
#define	_GP_HANG_DIAGNOSE			(FALSE & !(FANG_PRODUCTION_BUILD))
#define _GP_SHOW_PERFORMANCE		FALSE

#ifdef FANG_DEBUG_BUILD
//	#define _GX_WARNING_LEVEL	GX_WARN_ALL
//	#define _GX_WARNING_LEVEL	GX_WARN_MEDIUM
//	#define _GX_WARNING_LEVEL	GX_WARN_SEVERE
	#define _GX_WARNING_LEVEL	GX_WARN_NONE
#else
	#define _GX_WARNING_LEVEL	GX_WARN_NONE
#endif

typedef enum
{
	_LAST_BEGIN,
	_LAST_END,
	_LAST_SWAP,
	
} _LastCall_e;


// GameCube GP Tokens
#define _FIFO_DRAWDONE_TOKEN	0xBEEF
#define _FIFO_DRAWING_TOKEN		0xABCD
#define _FIFO_CLEAR_TOKEN		0xF00D

#define N_STATS         (sizeof(PerfStat)/sizeof(StatObj))


//////////////////////////////////////////////////////////////////////////////////////
// Global Variables:
//////////////////////////////////////////////////////////////////////////////////////

BOOL FVid_bOnline = FALSE;		// TRUE: Video system is online

f32  FVid_fBrightness;

// These are valid only when FVid_bOnline is TRUE:
FVidDev_t 	FVid_Dev;			// Current device
FVidMode_t 	FVid_Mode;			// Current video mode
FVidWin_t 	FVid_Win;			// Current window info

static FVidDev_t	_defaultVidDev;
static FVidMode_t	_aDefaultVidMode[3];

BOOL 	FVid_bBegin;				// TRUE: Inside a fvid_Begin()/fvid_End() pair
u32 	FVid_nFrameCounter;			// Increments each time fvid_Begin() is called
u32 	FVid_nSwapCounter;
BOOL 	FVid_bPrintHiFreqErrMsg;	// Set to TRUE for one frame every few seconds. Can be used to limit the number of error messages printed.

BOOL 	FGCvid_bResetting;			// TRUE: The device is being reset: only ->Release() can be used


#if _GP_SHOW_PERFORMANCE
typedef enum {
    GP_PERF0,
    GP_PERF1,
    MEM_PERF,
    PIX_PERF,
    VC_PERF
} PerfType;

//
//  Statistics
//
typedef struct {
    u32         cnt;  
    PerfType    stat_type;
    u32         stat;
    char        text[50];
} StatObj;

#define _CURR_STAT		0

StatObj PerfStat[] = { // stats to count
	{ 0, GP_PERF0, GX_PERF0_VERTICES,         "Vertices.........." },
//	{ 0, GP_PERF0, GX_PERF0_CLIP_VTX,         "num vtx clipped............" },
//	{ 0, GP_PERF0, GX_PERF0_CLIP_CLKS,        "num clipping clocks........" },
	{ 0, GP_PERF0, GX_PERF0_XF_WAIT_IN,       "xf awaiting input." },
	{ 0, GP_PERF0, GX_PERF0_XF_WAIT_OUT,      "xf awaiting output" },
	{ 0, GP_PERF0, GX_PERF0_XF_XFRM_CLKS,     "xf trans clocks..." },
	{ 0, GP_PERF0, GX_PERF0_XF_LIT_CLKS,      "xf light clocks..." },
//	{ 0, GP_PERF0, GX_PERF0_XF_BOT_CLKS,      "xf bot of pipe clocks......" },
//	{ 0, GP_PERF0, GX_PERF0_XF_REGLD_CLKS,    "xf register load clocks...." },
//	{ 0, GP_PERF0, GX_PERF0_XF_REGRD_CLKS,    "xf register read clocks...." },
//	{ 0, GP_PERF0, GX_PERF0_CLIP_RATIO,       "clip ratio................." },
	{ 0, GP_PERF0, GX_PERF0_TRIANGLES,        "Tris.............." },
//	{ 0, GP_PERF0, GX_PERF0_TRIANGLES_CULLED, "Tris culled......." },
	{ 0, GP_PERF0, GX_PERF0_TRIANGLES_PASSED, "Tris !culled......" },
//	{ 0, GP_PERF0, GX_PERF0_TRIANGLES_SCISSORED, "Tris scissored...." },
	{ 0, GP_PERF0, GX_PERF0_TRIANGLES_0TEX,   "Tris w/no tex....." },
	{ 0, GP_PERF0, GX_PERF0_TRIANGLES_1TEX,   "Tris w/1 tex......" },
	{ 0, GP_PERF0, GX_PERF0_TRIANGLES_2TEX,   "Tris w/2 tex......" },
	{ 0, GP_PERF0, GX_PERF0_TRIANGLES_3TEX,   "Tris w/3 tex......" },
	{ 0, GP_PERF0, GX_PERF0_TRIANGLES_4TEX,   "Tris w/4 tex......" },
//	{ 0, GP_PERF0, GX_PERF0_TRIANGLES_5TEX,   "Tris w/5 tex......" },
//	{ 0, GP_PERF0, GX_PERF0_TRIANGLES_6TEX,   "Tris w/6 tex......" },
//	{ 0, GP_PERF0, GX_PERF0_TRIANGLES_7TEX,   "Tris w/7 tex......" },
//	{ 0, GP_PERF0, GX_PERF0_TRIANGLES_8TEX,   "Tris w/8 tex......" },
//	{ 0, GP_PERF0, GX_PERF0_TRIANGLES_0CLR,   "Tris w/no clr....." },
//	{ 0, GP_PERF0, GX_PERF0_TRIANGLES_1CLR,   "Tris w/1 clr......" },
//	{ 0, GP_PERF0, GX_PERF0_TRIANGLES_2CLR,   "Tris w/2 clr......" },
//	{ 0, GP_PERF0, GX_PERF0_QUAD_0CVG,        "Quads w/0 coverage........." },
//	{ 0, GP_PERF0, GX_PERF0_QUAD_NON0CVG,     "Quads w/>0 coverage........" },
//	{ 0, GP_PERF0, GX_PERF0_QUAD_1CVG,        "Quads w/1 pix coverage....." },
//	{ 0, GP_PERF0, GX_PERF0_QUAD_2CVG,        "Quads w/2 pix coverage....." },
//	{ 0, GP_PERF0, GX_PERF0_QUAD_3CVG,        "Quads w/3 pix coverage....." },
//	{ 0, GP_PERF0, GX_PERF0_QUAD_4CVG,        "Quads w/4 pix coverage....." },
//	{ 0, GP_PERF0, GX_PERF0_AVG_QUAD_CNT,     "Average quad count........." },
//	{ 0, GP_PERF0, GX_PERF0_CLOCKS,           "Perf0 clocks..............." },

	{ 0, GP_PERF1, GX_PERF1_TEXELS,           "Number of texels.." },
	{ 0, GP_PERF1, GX_PERF1_TX_IDLE,          "Tex idle.........." },
//	{ 0, GP_PERF1, GX_PERF1_TX_REGS,          "Tex regs..................." },
	{ 0, GP_PERF1, GX_PERF1_TX_MEMSTALL,      "Tex mem stall....." },
//	{ 0, GP_PERF1, GX_PERF1_TC_CHECK1_2,      "Tex$ check 1,2............." },
//	{ 0, GP_PERF1, GX_PERF1_TC_CHECK3_4,      "Tex$ check 3,4............." },
//	{ 0, GP_PERF1, GX_PERF1_TC_CHECK5_6,      "Tex$ check 5,6............." },
//	{ 0, GP_PERF1, GX_PERF1_TC_CHECK7_8,      "Tex$ check 7,8............." },
	{ 0, GP_PERF1, GX_PERF1_TC_MISS,          "Tex$ miss........." },
//	{ 0, GP_PERF1, GX_PERF1_VC_ELEMQ_FULL,    "Vtx$ elem queue full......." },
//	{ 0, GP_PERF1, GX_PERF1_VC_MISSQ_FULL,    "Vtx$ miss queue full......." },
//	{ 0, GP_PERF1, GX_PERF1_VC_MEMREQ_FULL,   "Vtx$ mem request full......" },
//	{ 0, GP_PERF1, GX_PERF1_VC_STATUS7,       "Vtx$ status 7.............." },
//	{ 0, GP_PERF1, GX_PERF1_VC_MISSREP_FULL,  "Vtx$ miss replace full....." },
//	{ 0, GP_PERF1, GX_PERF1_VC_STREAMBUF_LOW, "Vtx$ stream buf low........" },
//	{ 0, GP_PERF1, GX_PERF1_VC_ALL_STALLS,    "Vtx$ all stalls............" },
//	{ 0, GP_PERF1, GX_PERF1_VERTICES,         "Perf1 vertices............." },
//	{ 0, GP_PERF1, GX_PERF1_FIFO_REQ,         "CP fifo requests..........." },
//	{ 0, GP_PERF1, GX_PERF1_CALL_REQ,         "CP call requests..........." },
//	{ 0, GP_PERF1, GX_PERF1_VC_MISS_REQ,      "Vtx$ miss request.........." },
//	{ 0, GP_PERF1, GX_PERF1_CP_ALL_REQ,       "CP all requests............" },
//	{ 0, GP_PERF1, GX_PERF1_CLOCKS,           "Perf1 clocks..............." },
//	{ 0, MEM_PERF, 0,                         "Memory requests............" }
//	{ 0, PIX_PERF, 0,                         "Pixel stats................" },
//	{ 0, VC_PERF,  0,                         "VCache stats..............." } 
};

#endif // _GP_SHOW_PERFORMANCE

#if _USE_TRIPLE_BUFFER
	/*---------------------------------------------------------------------------*
	 * Quick and dirty queue implementation.
	 *---------------------------------------------------------------------------*/

	typedef struct QItem_ 
	{
	    void* writePtr;
	    void* dataPtr;
	    void* copyXFB;
	} QItem;

	#define QUEUE_MAX 5
	#define QUEUE_EMPTY QUEUE_MAX

	typedef struct Queue_
	{
	    QItem entry[QUEUE_MAX];
	    u16 top;
	    u16 bot;
	} Queue;

	/*---------------------------------------------------------------------------*
	 * Data needed for triple-buffering.
	 *---------------------------------------------------------------------------*/

	static Queue RenderQ;   // Queue for frames in FIFO
	static Queue DoneQ;     // Queue for frames finished already

	static void* myXFB1;    // Pointers to the two XFB's
	static void* myXFB2;
	static void* copyXFB;   // Which XFB to copy to next
	static void* dispXFB;   // Which XFB is being displayed now

	static GXBool BPSet  = GX_FALSE;        // Is the FIFO breakpoint set?
	static GXBool BPWait = GX_FALSE;        // Is breakpt reset waiting on VBlank?
	static GXBool BPGo   = GX_FALSE;        // Indicates breakpt should be released

	static u16 lastVCBToken = 0;    // Last sync token the VBlank callback saw
	static u16 newToken = 1;        // Value to use for new sync token.

	static OSThreadQueue waitingDoneRender; // Threads waiting for frames to finish

	static OSThread CUThread;               // OS data for clean-up thread
	static u8       CUThreadStack[4096];    // Stack for clean-up thread

	void   BPCallback      ( void );
	void   VIPreCallback   ( u32 retraceCount );
	void   VIPostCallback  ( u32 retraceCount );
	void*  CleanupThread   ( void* param );
	void   SetNextBreakPt  ( void );

	void   init_queue  (Queue *q);
	void   enqueue     (Queue *q, QItem *qitm);
	QItem  dequeue     (Queue *q);
	QItem  queue_front (Queue *q);
	GXBool queue_empty (Queue *q);
	u32    queue_length(Queue *q);

	/*---------------------------------------------------------------------------*
	   Breakpoint Interrupt Callback
	 *---------------------------------------------------------------------------*/

	void BPCallback( void )
	{
	    QItem qitm;
	    
	    qitm = queue_front( &RenderQ );

	    // Check whether or not the just-finished frame can be
	    // copied already or if it must wait (due to lack of a
	    // free XFB).  If it must wait, set a flag for the VBlank
	    // interrupt callback to take care of it.

	    if ( qitm.copyXFB == dispXFB ) 
	    {
	        BPWait = GX_TRUE;
	    }
	    else
	    {
	        SetNextBreakPt();
	    }
	}

	/*---------------------------------------------------------------------------*
	   Routine to move breakpoint ahead, deal with finished frames.
	 *---------------------------------------------------------------------------*/

	void SetNextBreakPt( void )
	{
	    QItem qitm;

	    // Move entry from RenderQ to DoneQ.

	    qitm = dequeue( &RenderQ );

	    enqueue( &DoneQ, &qitm );

	    OSWakeupThread( &waitingDoneRender );

	    // Move breakpoint to next entry, if any.

	    if ( queue_empty( &RenderQ ) )
	    {
	        GXDisableBreakPt();
	        BPSet = GX_FALSE;
	    }
	    else
	    {
	        qitm = queue_front( &RenderQ );
	        GXEnableBreakPt( qitm.writePtr );
	    }
	}

	/*---------------------------------------------------------------------------*
	   VI Pre Callback (VBlank interrupt)

	   The VI Pre callback should be kept minimal, since the VI registers
	   must be set before too much time passes.  Additional bookkeeping is
	   done in the VI Post callback.

	 *---------------------------------------------------------------------------*/

	void VIPreCallback( u32 retraceCount )
	{
	    #pragma unused (retraceCount)
	    u16 token;

	    // We don't need to worry about missed tokens, since 
	    // the breakpt holds up the tokens, and the logic only
	    // allows one token out the gate at a time.

	    token = GXReadDrawSync();

		DEVPRINTF( "Read Token %d.\n", token );
		
	    // We actually need to use only 1 bit from the sync token.

	    if ( token == (u16)(lastVCBToken + 1) )
	    {
	        lastVCBToken = token;

	        dispXFB = (dispXFB == myXFB1) ? myXFB2 : myXFB1;

			DEVPRINTF( "DIPLAYING A FRAME.\n" );
	        VISetNextFrameBuffer( dispXFB );
	        VIFlush();

	        BPGo = GX_TRUE;
	    }
	}

	/*---------------------------------------------------------------------------*
	   VI Post Callback (VBlank interrupt)
	 *---------------------------------------------------------------------------*/

	void VIPostCallback( u32 retraceCount )
	{
	    #pragma unused (retraceCount)

	    if (BPWait && BPGo)
	    {
	        SetNextBreakPt();
	        BPWait = GX_FALSE;
	        BPGo = GX_FALSE;
	    }
	}

	/*---------------------------------------------------------------------------*
	   Cleanup Thread
	 *---------------------------------------------------------------------------*/

	void* CleanupThread( void* param )
	{
	    #pragma unused (param)
	    QItem qitm;

	    while(1) {

	        OSSleepThread( &waitingDoneRender );
	        
	        qitm = dequeue( &DoneQ );

	        // Take qitm.dataPtr and do any necessary cleanup.
	        // That is, free up any data that only needed to be
	        // around for the GP to read while rendering the frame.
	    }
	}

	/*---------------------------------------------------------------------------*
	 * Quick and dirty queue implementation.
	 *---------------------------------------------------------------------------*/

	void init_queue(Queue *q)
	{
	    q->top = QUEUE_EMPTY;
	}

	void enqueue(Queue *q, QItem *qitm)
	{
	    if (q->top == QUEUE_EMPTY) 
	    {
	        q->top = q->bot = 0;
	    }
	    else
	    {
	        q->top = (u16) ((q->top + 1) % QUEUE_MAX);
	        
	        OSReport( "Queue Top: %d - Bottom: %d\n", q->top, q->bot );
	    
	        if (q->top == q->bot) 
	        {   // error, overflow
	            OSHalt("queue overflow");
	        }
	    }
	    
	    q->entry[q->top] = *qitm;
	}

	QItem dequeue(Queue *q)
	{
	    u16 bot = q->bot;

	    if (q->top == QUEUE_EMPTY)
	    {   // error, underflow
	        OSHalt("queue underflow");
	    }
	    
	    if (q->bot == q->top) 
	    {
	        q->top = QUEUE_EMPTY;
	    }
	    else
	    {
	        q->bot = (u16) ((q->bot+1) % QUEUE_MAX);
	    }

	    return q->entry[bot];
	}

	QItem queue_front(Queue *q)
	{
	    if (q->top == QUEUE_EMPTY)
	    {   // error, queue empty
	        OSHalt("queue_top: queue empty");
	    }

	    return q->entry[q->bot];
	}

	GXBool queue_empty(Queue *q)
	{
	    return q->top == QUEUE_EMPTY;
	}

	u32 queue_length(Queue *q)
	{
	    if ( q->top == QUEUE_EMPTY ) 
	    {
	    	OSReport( "Queue is empty!\n" );
	    	return 0;
	    }

	    if (q->top >= q->bot)
	        return (u32) ((s32) q->top - q->bot + 1);
	    else
	        return (u32) ((s32) (q->top + QUEUE_MAX) - q->bot + 1);
	}
#endif

// GameCube frame buffers
void *FGCVid_pFrameBuffer1 		= NULL;
void *FGCVid_pFrameBuffer2 		= NULL;
void *FGCVid_pCurrentBuffer 	= NULL;
u32  FGCVid_nFrameBufferSize 	= 0;
void *FGCVid_pGraphicsFifo		= NULL;
u32  FGCVid_nGraphicsFifoSize 	= 0;


// GX Render mode objects
GXRenderModeObj *FGCVid_pRenderMode = NULL;
GXRenderModeObj FGCVid_RenderModeObject;

// GX Mode FIFO
GXFifoObj	*FGCVid_pDefaultFifoObj = NULL;


#define _MAX_WINDOW_CREATION_CALLBACK_FCNS	20

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }



//////////////////////////////////////////////////////////////////////////////////////
// Local Variables:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL 		_bModuleInitialized;
static FResFrame_t 	_MasterResFrame;
static _LastCall_e	_bLastDrawFuncCalled;

static int _nWindowCallbackCount;
static FGCVidWindowCallback_t **_ppFcnWindowCallbackTable;

static FVidDrawOverlayFcn_t *_pFcnDrawOverlay;

// Enumeration:
static BOOL 			_bEnumerated;
static FVidRenderer_t 	_nRenderer;

// Current video mode:

// Window operation:
static volatile BOOL _bReady;
static volatile BOOL _bDeviceLost;
static volatile BOOL _bResetFailed;
static BOOL			 _bBlackFrameOn;

static u32 _nCurrStat = 0;



//////////////////////////////////////////////////////////////////////////////////////
// Local Functions:
//////////////////////////////////////////////////////////////////////////////////////

static void	_ClearData( void );
static void _ClearVideoData( void );
static BOOL _SwapBuffers( void );
static BOOL _CreateWindow( void );
static int 	_FindWindowCallbackFunction( FGCVidWindowCallback_t *pFcnWindowCallback );
static BOOL _CallWindowCallbackFunctions( FGCVidEvent_e nEvent );

static void _FifoTick( void );

#if _GP_HANG_DIAGNOSE
	// Functions and vars to diagnose GP hangs
	static void _SetGPHangMetric( GXBool enable );
	static void _NoHangRetraceCallback ( u32 count );
	static void _DiagnoseHang( void );
	static u32 _nFrameMissThreshold;
	static u32 _nHangFrameCount = 0;
#endif

static f32 _fAverageFPS = 0.f;


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
	_ppFcnWindowCallbackTable = (FGCVidWindowCallback_t **)fres_Alloc( sizeof(FGCVidWindowCallback_t *) * _MAX_WINDOW_CREATION_CALLBACK_FCNS );
	if ( _ppFcnWindowCallbackTable == NULL ) 
		return FALSE;

	_defaultVidDev.szName[0] = 'G';
	_defaultVidDev.szName[1] = 'C';
	_defaultVidDev.szName[2] = 0;
	_defaultVidDev.nFlags = FVID_DEVFLAG_HW_TNL;
	_defaultVidDev.nOrdinal = 0;
	_defaultVidDev.nRenderer = FVID_RENDERER_HARDWARE;

	_aDefaultVidMode[0].nColorBits = 24;
	_aDefaultVidMode[0].nDepthBits = 24;
	_aDefaultVidMode[0].nStencilBits = 0;
	_aDefaultVidMode[0].nPixelsAcross = 640;
	_aDefaultVidMode[0].nPixelsDown = 480;

	_aDefaultVidMode[1].nColorBits = 16;
	_aDefaultVidMode[1].nDepthBits = 16;
	_aDefaultVidMode[1].nStencilBits = 0;
	_aDefaultVidMode[1].nPixelsAcross = 640;
	_aDefaultVidMode[1].nPixelsDown = 480;
	
//	_aDefaultVidMode[2].nColorBits = 32;
//	_aDefaultVidMode[2].nDepthBits = 16;
//	_aDefaultVidMode[2].nStencilBits = 0;
//	_aDefaultVidMode[2].nPixelsAcross = 640;
//	_aDefaultVidMode[2].nPixelsDown = 480 * 2;

	FGCvid_bResetting = FALSE;
	_bModuleInitialized = TRUE;

	_pFcnDrawOverlay = NULL;
	
	FVid_nSwapCounter = 0;
	
	_bBlackFrameOn = TRUE;

	// Initialize the GP FIFO
#if _USE_BUFFERED_FIFO
	FGCVid_pDefaultFifoObj = GXInit( FGCVid_pGraphicsFifo, FGCVid_nGraphicsFifoSize );
	
	// So that the first fifo will fire off immediately
	GXSetDrawSync( _FIFO_DRAWDONE_TOKEN );
#else
	FGCVid_pDefaultFifoObj = GXInit( FGCVid_pGraphicsFifo, FGCVid_nGraphicsFifoSize );
#endif // _USE_BUFFERED_FIFO

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

	fang_Free( _ppFcnWindowCallbackTable );

	_ClearData();

	_bModuleInitialized = FALSE;
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
	FASSERT( _bModuleInitialized );
	FASSERT( pWinInfo );
	
	fvid_DestroyWindow();
	_pFcnDrawOverlay = NULL;

	FVid_Win = *pWinInfo;
	
	if ( _CreateWindow() ) 
	{
		fgc_InitRenderState();
	
		// Window creation succeeded so call the window callback functions
		if ( _CallWindowCallbackFunctions( FGCVID_EVENT_WINDOW_CREATED ) ) 
		{
			return TRUE;
		}
			
		// If any of the the window callback functions failed, bail out

		_bReady = FALSE;
		FVid_bOnline = FALSE;

		_ClearVideoData();
		
		return FALSE;
	} 
	else 
	{
		// Window creation failed so we're screwed
		return FALSE;
	}
}


//
//
//
BOOL fvid_ResetWindow( const FVidWin_t *pWinInfo ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( pWinInfo );
	FASSERT( _bLastDrawFuncCalled == _LAST_END );
	
	_pFcnDrawOverlay = NULL;

	FVid_Win = *pWinInfo;
	
	fvid_Swap();
	
	VIFlush();
	VIWaitForRetrace();
	
	GXDrawDone();
	
	VIFlush();
	VIWaitForRetrace();
	
	if ( _CreateWindow() ) 
	{
		return TRUE;
	} 
	
	// Window creation failed so we're screwed
	_bReady = FALSE;
	FVid_bOnline = FALSE;

	_ClearVideoData();
	
	return FALSE;
}


//
//
//
void fvid_DestroyWindow( void ) 
{
	FASSERT( _bModuleInitialized );

	if ( FVid_bOnline ) 
	{
		_CallWindowCallbackFunctions( FGCVID_EVENT_WINDOW_DESTROYED );

		if( FVid_bBegin ) 
		{
			fvid_End();
		}

		GXAbortFrame();
		
		VISetBlack( TRUE );
		VIFlush();
		VIWaitForRetrace();
		
		_bReady = FALSE;
		FVid_bOnline = FALSE;

		_ClearVideoData();
	}
}


//
//
//
BOOL fvid_IsMinimized( void ) 
{
	FASSERT( _bModuleInitialized );

	return FALSE;
}


//
//
//
void fGCvid_Flush( BOOL bBlock ) 
{
	if ( _bLastDrawFuncCalled == _LAST_END )
	{
		fvid_Swap();
	}
	else if ( _bLastDrawFuncCalled == _LAST_BEGIN )
	{
		fvid_End();
		fvid_Swap();
	}
}


u32 iTemp = 0;
//
//
//
BOOL fvid_Begin( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );
	FASSERT( !FVid_bBegin );
//	FASSERT( _bLastDrawFuncCalled == _LAST_SWAP );

	FVid_bBegin = TRUE;
	
#if _USE_BUFFERED_FIFO
	void *readPtr, *writePtr;

	GXFlush();
	
	// Set a breakpoint at the current point in the CPU 
	// and disable the previous one to let the GP start processing
	GXGetFifoPtrs( GXGetCPUFifo(), &readPtr, &writePtr );
	GXEnableBreakPt( writePtr );

	GXSetDrawSync( _FIFO_DRAWING_TOKEN );
#endif // _USE_BUFFERED_FIFO
		
#if _GP_SHOW_PERFORMANCE
	//
	//  Enable various statistics
	//  clear counters
	//
	if (PerfStat[_nCurrStat].stat_type == GP_PERF0)
	{
		GXSetGP0Metric((GXPerf0)PerfStat[_nCurrStat].stat);
		GXClearGP0Metric();
	}
	else if (PerfStat[_nCurrStat].stat_type == GP_PERF1)
	{
		GXSetGP1Metric((GXPerf1)PerfStat[_nCurrStat].stat);
		GXClearGP1Metric();
	}
	else if (PerfStat[_nCurrStat].stat_type == MEM_PERF)
	{
		GXClearMemMetric();
	}
	else if (PerfStat[_nCurrStat].stat_type == PIX_PERF)
	{
		GXClearPixMetric();
	}
	else // vcache stat
	{
		GXSetVCacheMetric(GX_VC_ALL);
		GXClearVCacheMetric();
	}
#endif // _GP_SHOW_PERFORMANCE

	fcolor_GenerateMotifs();
	fvid_IncrementFrameCounter();
	fmesh_ResetLightList();

	// Set up viewport
	if ( FGCVid_pRenderMode->field_rendering )
	{
		GXSetViewportJitter( 0.f, 0.f, (float)FGCVid_pRenderMode->fbWidth, 
			(float)FGCVid_pRenderMode->xfbHeight, 0.f, 1.f, VIGetNextField() );
	}
//	else
//	{
//		GXSetViewport( 0.f, 0.f, (float)FGCVid_pRenderMode->fbWidth, 
//			(float)FGCVid_pRenderMode->xfbHeight, 0.f, 1.f );
//	}
    
	// Invalidate vertex cache in GP
	GXInvalidateVtxCache();

	// Invalidate texture cache in GP
//	GXInvalidateTexAll();

	_bLastDrawFuncCalled = _LAST_BEGIN;
	
	return TRUE;
}

//
//
//
void fvid_End( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );
	FASSERT( _bLastDrawFuncCalled == _LAST_BEGIN );

	if ( FVid_bBegin ) 
	{
	#if _SHOW_FRAMERATE
			ftext_DebugPrintf( 0.5f, 0.075f, "~w1~alFPS %.02f", _fAverageFPS );
	#endif // _SHOW_FRAMERATE

	#if (FANG_PRODUCTION_BUILD & FANG_ENABLE_DEV_FEATURES)
		if ( FGC_DEVKitInExtendedMemoryMode )
		{
			f32 fAvailableMB = (f32)FRes_CFHeap.GetFreeBytes() / (1024.f * 1024.f);
			if ( fAvailableMB - 8 > 0 )
			{
				ftext_DebugPrintf( 0.5f, 0.20f, "~w1~acMEMORY LEFT: %5.3f MB", fAvailableMB - 8 );
			}
			else
			{
				ftext_DebugPrintf( 0.5f, 0.20f, "~s1.50~c99111199~w1~acMEMORY OVER: %5.3f MB", fmath_Abs(fAvailableMB - 8) );
			}
		}
	#endif

	fperf_RenderPerf();
	
#if !FANG_PRODUCTION_BUILD
	#if FHEAP_DETAILED_TRACKING
		if ( FVid_nSwapCounter == 2000 )
		{
//			fheap_ShowMemoryTracking( TRUE, TRUE, TRUE, FALSE, NULL );
		}
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
#endif // !FANG_PRODUCTION_BUILD
	
		ftext_Draw(); 
		
		if ( _pFcnDrawOverlay ) 
		{
			_pFcnDrawOverlay();
		}

		FVid_bBegin = FALSE;
	}
	
	_bLastDrawFuncCalled = _LAST_END;
}


//
//
//
BOOL fvid_Swap( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );
	FASSERT( !FVid_bBegin );
	
	if ( _bLastDrawFuncCalled != _LAST_END )
	{
		return TRUE;
	}

	frenderer_PopAll();
	
	fgcxfm_Swap();
	fgcvb_Swap();
	fgcsh_Swap();

	_bLastDrawFuncCalled = _LAST_SWAP;
	
	return _SwapBuffers();
}


//
//
//
void fvid_Flush( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVid_bOnline );
	FASSERT( !FVid_bBegin );
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
	if ( !(FVid_nFrameCounter & 63) ) 
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
	// TODO
	return FALSE;
}


//
// fUnitGamma can range from 0.0f to 1.0f (0.5f is default)
//
void fvid_SetGamma( f32 fUnitGamma ) 
{
	return;
}


//
//
//
void fgcvid_RegisterWindowCallbackFunction( FGCVidWindowCallback_t *pFcnWindowCallback ) 
{
	FASSERT( _bModuleInitialized );

	if( pFcnWindowCallback == NULL ) 
		return;

	FASSERT( _FindWindowCallbackFunction( pFcnWindowCallback ) == -1 );
	FASSERT( _nWindowCallbackCount < _MAX_WINDOW_CREATION_CALLBACK_FCNS );

	_ppFcnWindowCallbackTable[_nWindowCallbackCount++] = pFcnWindowCallback;
}


//
//
//
void fgcvid_UnregisterWindowCallbackFunction( FGCVidWindowCallback_t *pFcnWindowCallback ) 
{
	int i, nIndex;

	FASSERT( _bModuleInitialized );

	if( pFcnWindowCallback == NULL ) {
		return;
	}

	nIndex = _FindWindowCallbackFunction( pFcnWindowCallback );

	if ( nIndex != -1 ) 
	{
		// Function found...

		for( i=nIndex+1; i<_nWindowCallbackCount; i++ ) 
			_ppFcnWindowCallbackTable[i-1] = _ppFcnWindowCallbackTable[i];

		_nWindowCallbackCount--;
	}
}


//
//
//
BOOL fgcvid_IsWindowCallbackFunctionRegistered( FGCVidWindowCallback_t *pFcnWindowCallback ) 
{
	FASSERT( _bModuleInitialized );

	if ( pFcnWindowCallback == NULL )
		return TRUE;

	if ( _FindWindowCallbackFunction( pFcnWindowCallback ) != -1 ) 
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
static void _ClearData( void ) 
{
	FVid_bOnline = FALSE;

	_bEnumerated = FALSE;

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

	FVid_bBegin = FALSE;
	FVid_nFrameCounter = 0;
	FVid_bPrintHiFreqErrMsg = FALSE;
}



//
//
//
static BOOL _CreateWindow( void ) 
{
//	FASSERT( !FVid_bOnline );

	// Don't even bother validating - assume they passed in the same as they got...
	FVid_Dev 	= _defaultVidDev;
	FVid_Mode 	= _aDefaultVidMode[0];

	FGCVid_RenderModeObject = GXNtsc480IntDf;
	FGCVid_pRenderMode = &FGCVid_RenderModeObject;

	// RAF -- Note: These values are 'tuned' to have the same overscan characteristics
	// as the XBOX...  
	FGCVid_RenderModeObject.fbWidth = 512;
	FGCVid_RenderModeObject.efbHeight = 464;
	FGCVid_RenderModeObject.xfbHeight = 464;
	FGCVid_RenderModeObject.viXOrigin = 8; //RAF -- used to be 45
	FGCVid_RenderModeObject.viYOrigin = 0;
	FGCVid_RenderModeObject.viWidth = 710; //RAF -- used to be 640
	FGCVid_RenderModeObject.viHeight = 464;
	
	if ( VIGetTvFormat() == VI_MPAL )
	{
		FGCVid_RenderModeObject.xfbHeight = 550;
		FGCVid_RenderModeObject.viHeight = 550;
		FGCVid_RenderModeObject.viTVmode = VI_TVMODE_MPAL_INT;
	}
	else if ( fsysinfo_GetCapabilities() & FSYSINFO_VIDEO_PAL )
	{
		if ( FVid_Win.VidMode.nFlags & FVID_MODEFLAG_PAL_60HZ )
		{
			// PAL 60 Hz. mode uses a 640x480 screen just like NTSC
			FGCVid_RenderModeObject.viTVmode = VI_TVMODE_EURGB60_INT;
		}
		else
		{
			// PAL 50 Hz requires a rescaling of the render to match the PAL 575 vertical resolution
			FGCVid_RenderModeObject.xfbHeight = 550;
			FGCVid_RenderModeObject.viHeight = 550;
			FGCVid_RenderModeObject.viTVmode = VI_TVMODE_PAL_INT;
		}
	}
	else if ( fsysinfo_GetCapabilities() & FSYSINFO_VIDEO_NTSC )
	{
		if ( fsysinfo_GetCapabilities() & FSYSINFO_VIDEO_480P )
		{
			FGCVid_RenderModeObject.viTVmode = VI_TVMODE_NTSC_PROG;
		}
		else
		{
			FGCVid_RenderModeObject.viTVmode = VI_TVMODE_NTSC_INT;
		}
	}
	else
	{
		DEVPRINTF( "_CreateWindow(): ERROR - Invalid TV format\n" );
		FASSERT_NOW;
		return FALSE;
	}

	VIConfigure( FGCVid_pRenderMode );

	// GX configuration by a render mode obj

	// These are all necessary function calls to take a render mode
	// object and set up the relevant GX configuration.

	// Note the use of "xfbHeight" in the line below.  This is used
	// instead of "efbHeight" for compatibility with full-frame AA mode.
	// If you use a scaled video mode, "efbHeight" may be appropriate.
	GXSetViewport( 0.f, 0.f, (f32)FGCVid_pRenderMode->fbWidth, (f32)FGCVid_pRenderMode->xfbHeight, 0.f, 1.f );
	GXSetScissor( 0, 0, (u32)FGCVid_pRenderMode->fbWidth, (u32)FGCVid_pRenderMode->efbHeight );
//	GXSetScissorBoxOffset( 0.f, 16.f );
	
	f32 yScale = GXGetYScaleFactor( FGCVid_pRenderMode->efbHeight, FGCVid_pRenderMode->xfbHeight );
	u16 xfbHeight = (u16)GXSetDispCopyYScale( yScale );
	GXSetDispCopySrc( 0, 0, FGCVid_pRenderMode->fbWidth, FGCVid_pRenderMode->efbHeight );
	GXSetDispCopyDst( FGCVid_pRenderMode->fbWidth, xfbHeight );
	GXSetCopyFilter( FGCVid_pRenderMode->aa, FGCVid_pRenderMode->sample_pattern, GX_TRUE, FGCVid_pRenderMode->vfilter );
	
	// Note that this scale is inappropriate for full-frame AA mode!
	GXSetDispCopyYScale( (f32)(FGCVid_pRenderMode->xfbHeight) / (f32)(FGCVid_pRenderMode->efbHeight) ); 

	if ( FGCVid_pRenderMode->aa )
	{
		FDraw_nDepthBitDepth = 16;
		GXSetPixelFmt( GX_PF_RGB565_Z16, GX_ZC_LINEAR );
	}
	else
	{
		FDraw_nDepthBitDepth = 24;
		GXSetPixelFmt( GX_PF_RGB8_Z24, GX_ZC_LINEAR );
//		GXSetPixelFmt( GX_PF_RGBA6_Z24, GX_ZC_LINEAR );
		fgc_SetDitherMode( FALSE );
	}

	GXSetMisc( GX_MT_XF_FLUSH, GX_XF_FLUSH_SAFE );

	GXColor CopyClear;
	CopyClear.r = 0;
	CopyClear.g = 0;
	CopyClear.b = 0;
	CopyClear.a = 255;
	GXSetCopyClear( CopyClear, GX_MAX_Z24 );
	
	_bBlackFrameOn = TRUE;
	VISetBlack( TRUE );

    // Clear embedded framebuffer for the first frame
	GXCopyDisp( FGCVid_pCurrentBuffer, GX_TRUE );

	// Set GX Warning level
	GXSetVerifyLevel( _GX_WARNING_LEVEL );

	FGCVid_nFrameBufferSize = VIPadFrameBufferWidth( FGCVid_pRenderMode->fbWidth ) * FGCVid_pRenderMode->xfbHeight * VI_DISPLAY_PIX_SZ;

	// Gamma correction
	//
	GXSetDispCopyGamma( GX_GM_1_0 );

	u32 nin;

#if _GP_HANG_DIAGNOSE
	fgcvid_EnableGPHangDiagnose( 30 );
#endif

	// Double-buffering initialization
	VISetNextFrameBuffer( FGCVid_pFrameBuffer2 );
	FGCVid_pCurrentBuffer = FGCVid_pFrameBuffer1;

	// Tell VI device driver to write the current VI settings so far
	VIFlush();
    
	// Wait for retrace to start first frame
	VIWaitForRetrace();

	// Because of hardware restriction, we need to wait one more 
	// field to make sure mode is safely changed when we change
	// INT->DS or DS->INT. (VIInit() sets INT mode as a default)
	nin = (u32)FGCVid_pRenderMode->viTVmode & 1;
	if (nin)
	{
		VIWaitForRetrace();
	}

#if _USE_TRIPLE_BUFFER
    init_queue(&RenderQ);
    init_queue(&DoneQ);

    OSInitThreadQueue( &waitingDoneRender );

    // Creates a new thread. The thread is suspended by default.
    OSCreateThread(
        &CUThread,                          // ptr to the thread to init
        CleanupThread,                      // ptr to the start routine
        0,                                  // param passed to start routine
        CUThreadStack+sizeof(CUThreadStack),// initial stack address
        sizeof CUThreadStack,
        14,                                 // scheduling priority
        OS_THREAD_ATTR_DETACH);             // detached by default

    // Starts the thread
    OSResumeThread(&CUThread);

    myXFB1 = FGCVid_pFrameBuffer1;
    myXFB2 = FGCVid_pFrameBuffer2;
    dispXFB = myXFB2;
    copyXFB = myXFB1;

    (void) VISetPreRetraceCallback(VIPreCallback);
    (void) VISetPostRetraceCallback(VIPostCallback);
    (void) GXSetBreakPtCallback(BPCallback);
#endif

	// Window is ready to go so set the relevant Fang vars
	_bResetFailed = FALSE;
	_bReady = TRUE;
	FVid_bOnline = TRUE;

	FVid_bBegin = FALSE;
//	FVid_nFrameCounter = 1;

	_bLastDrawFuncCalled = _LAST_END;

	return TRUE;
}

//
// Returns TRUE if successful, or FALSE if a catastrophic error
// occurred. If a catastrophic error is reported, the game has
// no choice but to shut down without any further graphics
// processing.
//
static BOOL _SwapBuffers( void ) 
{
#if _SHOW_FRAMERATE
	#define MAX_FRAME_TIME_MEASURE	1000
	static f32 __fFrameTime[MAX_FRAME_TIME_MEASURE];
	static u32 __nAccFrames = 0;
	static f32 __fAccFrameSec = 0;
	static f32 __fLastFrameSec = 0;
	static f32 __fLastFPSReport = 0;
#endif

	// Make sure the Zbuffer and Color buffer are cleared in the new frame buffer
	GXSetZMode( GX_DISABLE, GX_ALWAYS, GX_ENABLE );

#if _USE_TRIPLE_BUFFER	
	    void* tmp_read;
	    void* tmp_write;
	    QItem qitm;
	    int   enabled;
	    
		enabled = OSDisableInterrupts();
		OSReport( "Length Pre %d\n", RenderQ.top );
		u32 nQueLength = queue_length( &RenderQ );
		OSReport( "Length Post %d\n", nQueLength );
		if ( queue_length( &RenderQ ) > 1 ) 
		{
			OSSleepThread( &waitingDoneRender );
		}
		OSRestoreInterrupts( enabled );
		GXGetFifoPtrs(GXGetCPUFifo(), &tmp_read, &tmp_write);

		// Create new render queue item
		qitm.writePtr = tmp_write;
		qitm.dataPtr = NULL;        // pointer to frame-related user data
		qitm.copyXFB = copyXFB;
        
		// Technically, you can work this such that you don't
		// need the OSDisabled interrupts.  You need to rework
		// the enqueue/dequeue routines a bit, though, to make
		// them non-interfere with each other.

		enabled = OSDisableInterrupts();
		enqueue( &RenderQ, &qitm );
		OSRestoreInterrupts(enabled);

		if ( BPSet == GX_FALSE ) 
		{
			BPSet = GX_TRUE;
			GXEnableBreakPt( tmp_write );
		}

		DEVPRINTF( "Inserting Token %d.\n", newToken );
		GXSetDrawSync( newToken );
		GXCopyDisp( copyXFB, GX_TRUE);
		GXFlush();

		// If this is the first frame, turn off VIBlack
		if ( _bBlackFrameOn == TRUE )
		{
			_bBlackFrameOn = FALSE;
			VISetBlack( FALSE );
		}

        newToken++;
        
#else // _USE_TRIPLE_BUFFER

	#if _GP_HANG_DIAGNOSE
		// Issue display copy command
		GXCopyDisp( FGCVid_pCurrentBuffer, GX_TRUE );
	
		GXSetDrawSync( _FIFO_DRAWDONE_TOKEN );
		
		_nHangFrameCount = 0;

		while( GXReadDrawSync() != _FIFO_DRAWDONE_TOKEN )
		{
	        if (_nHangFrameCount >= _nFrameMissThreshold)
	        {
	            OSReport("---------WARNING : GP HANG DETECTED ----------\n");
				_DiagnoseHang();
				FASSERT_NOW;
				// Could reinit the video system and continue
	        }
	    }
	#elif _USE_BUFFERED_FIFO
		// copy out the other framebuffer since GP is a frame behind
		if( FGCVid_pCurrentBuffer == FGCVid_pFrameBuffer1 )
			GXCopyDisp(FGCVid_pFrameBuffer2, GX_TRUE);
		else
			GXCopyDisp(FGCVid_pFrameBuffer1, GX_TRUE);

		GXSetDrawSync( _FIFO_DRAWDONE_TOKEN );
	    
		// wait until GP is finished by polling for the drawdone token in current GP Fifo
		while( (GXReadDrawSync()) != _FIFO_DRAWDONE_TOKEN )
		{        
		}
		
	#else
		// Wait until everything is drawn and copied into XFB.
		GXDrawDone();
		
		u8 nNewFilter[7];
		f32 fBrightness = FVid_fBrightness * 1.1f;
		FMATH_CLAMP( fBrightness, 0.f, 2.f );

		u32 i;
		for ( i = 0; i < 7; ++i )
		{
			nNewFilter[i] = u8( FGCVid_pRenderMode->vfilter[i] * fBrightness );
		}

		GXSetCopyFilter( FGCVid_pRenderMode->aa, FGCVid_pRenderMode->sample_pattern, GX_TRUE, nNewFilter );
		GXCopyDisp( FGCVid_pCurrentBuffer, GX_TRUE );
		GXSetCopyFilter( FGCVid_pRenderMode->aa, FGCVid_pRenderMode->sample_pattern, GX_TRUE, FGCVid_pRenderMode->vfilter );

		// Issue display copy command
//		GXCopyDisp( FGCVid_pCurrentBuffer, GX_TRUE );
	#endif

	// If this is the first frame, turn off VIBlack
	if ( _bBlackFrameOn == TRUE )
	{
		_bBlackFrameOn = FALSE;
		VISetBlack( FALSE );
	}

	// Display the buffer which was just filled by GXCopyDisp()
	VISetNextFrameBuffer( FGCVid_pCurrentBuffer);

	// Swap buffers
	if ( FGCVid_pCurrentBuffer == FGCVid_pFrameBuffer1 )
	{
		FGCVid_pCurrentBuffer = FGCVid_pFrameBuffer2;
	}
	else
	{
		FGCVid_pCurrentBuffer = FGCVid_pFrameBuffer1;
	}

	FVid_nSwapCounter++;

	#if !_OVERRIDE_SYNC_TO_VBLANK
		// Wait for vertical retrace.
	#if !FANG_PRODUCTION_BUILD
		if ( fmovie2_GetStatus() == FMOVIE2_STATUS_PLAYING )
		{
			VIWaitForRetrace();
			
			// Tell VI device driver to write the current VI settings to this point
			VIFlush();
		}
		else
		{
			if ( FVid_Win.nSwapInterval )
			{
				for ( u32 i = 0; i < FVid_Win.nSwapInterval; i++ )
				{
					VIWaitForRetrace();
				}
			}
			
			// Tell VI device driver to write the current VI settings to this point
			VIFlush();
		}
	#else
		static f32 __fPriorCurrentSeconds;
		if ( fmovie2_GetStatus() == FMOVIE2_STATUS_PLAYING )
		{
			VIWaitForRetrace();
			
			// Tell VI device driver to write the current VI settings to this point
			VIFlush();
		}
		else
		{
			// Tell VI device driver to write the current VI settings to this point
			VIFlush();
		
			f32 fCurrentSeconds = ftimer_Clock_GetSeconds();
			if ( fCurrentSeconds - __fPriorCurrentSeconds < 0.016666667f )
			{
				// If we're running at > 60 fps, then wait for a retrace
				VIWaitForRetrace();
			}
		}
		__fPriorCurrentSeconds = ftimer_Clock_GetSeconds();
	#endif	
	#else
	/*
		// If we're not sync'ing to the VBlank, make sure we don't overrun the FIFO
		GXBool bOverHi, bUnderLo, bCPUWrite, bGPRead, bWrap;
		u32 nNumCacheLines;
		GXGetFifoStatus( FGCVid_pDefaultFifoObj, &bUnderLo, &bOverHi, &nNumCacheLines, &bCPUWrite, &bGPRead, &bWrap );
		while ( bOverHi )
		{
			__bFifoOverHi[ __nAccFrames ]++;
			GXGetFifoStatus( FGCVid_pDefaultFifoObj, &bUnderLo, &bOverHi, &nNumCacheLines, &bCPUWrite, &bGPRead, &bWrap );
		}
	*/
	#endif // _OVERRIDE_SYNC_TO_VBLANK

#endif // _USE_TRIPLE_BUFFER

	// Set the ztest back to the current setting
	GXSetZMode( FGC_bZRead, FGC_ZCompareMode, FGC_bZWrite );

#if _SHOW_FRAMERATE
	f32 fSeconds = ftimer_Clock_GetSeconds();
	if ( __nAccFrames < MAX_FRAME_TIME_MEASURE )
	{
		__fFrameTime[ __nAccFrames ] = fSeconds - __fLastFrameSec;
	}
	__fAccFrameSec += fSeconds - __fLastFrameSec;
	__fLastFrameSec = fSeconds;
	__nAccFrames++;
	
	if ( fSeconds -  __fLastFPSReport >= 1.f )
	{
		_fAverageFPS = (f32)__nAccFrames / __fAccFrameSec;
		
		__fLastFPSReport = fSeconds;
		__nAccFrames = 0.f;
		__fAccFrameSec = 0.f;
	}
#endif // SHOW_FRAMERATE

#if _GP_SHOW_PERFORMANCE
	// pixel statistics counters
	static u32 topPixIn;
	static u32 topPixOut;
	static u32 botPixIn;
	static u32 botPixOut;
	static u32 clrPixIn;
	static u32 copyClks;
	
	// vcache statistics counters
	static u32 vcCheck;
	static u32 vcMiss;
	static u32 vcStall;
	
	// clocks per verts
	static u32 cpReq, tcReq, cpuRdReq, cpuWrReq, dspReq, ioReq, viReq, peReq, rfReq, fiReq;
	
	//
	//  Read back various statistics
	//
	if (PerfStat[_nCurrStat].stat_type == GP_PERF0)
	{
		PerfStat[_nCurrStat].cnt = GXReadGP0Metric();
	}
	else if (PerfStat[_nCurrStat].stat_type == GP_PERF1)
	{
		PerfStat[_nCurrStat].cnt = GXReadGP1Metric();
	}
	else if (PerfStat[_nCurrStat].stat_type == MEM_PERF)
	{
		GXReadMemMetric( &cpReq, &tcReq, &cpuRdReq, &cpuWrReq, &dspReq, &ioReq, &viReq, &peReq, &rfReq, &fiReq );
	}
	else if (PerfStat[_nCurrStat].stat_type == PIX_PERF)
	{
		GXReadPixMetric( &topPixIn, &topPixOut, &botPixIn, &botPixOut, &clrPixIn, &copyClks );
	}
	else // vcache stats
	{
		GXReadVCacheMetric( &vcCheck, &vcMiss, &vcStall );
	}

	char szText[128];
	for ( i = 0; i < N_STATS; i++ )
	{
		if (PerfStat[i].stat_type == PIX_PERF)
		{
			OSReport("Top pixels in..............:   %8d\n", topPixIn);
			OSReport("Top pixels out.............:   %8d\n", topPixOut);
			OSReport("Bot pixels in..............:   %8d\n", botPixIn);
			OSReport("Bot pixels out.............:   %8d\n", botPixOut);
			OSReport("Clr pixels in..............:   %8d\n", clrPixIn);
			OSReport("Copy clocks................:   %8d\n", copyClks);
		}
		else if (PerfStat[i].stat_type == VC_PERF)
		{
			OSReport("Vcache checks..............:   %8d\n", vcCheck);
			OSReport("Vcache misses..............:   %8d\n", vcMiss);
			OSReport("Vcache stalls..............:   %8d\n", vcStall);
		}
		else if (PerfStat[i].stat_type == MEM_PERF)
		{
			OSReport("CP requests................:   %8d\n", cpReq);
			OSReport("TC requests................:   %8d\n", tcReq);
			OSReport("CPU Rd requests............:   %8d\n", cpuRdReq);
			OSReport("CPU Wr requests............:   %8d\n", cpuWrReq);
			OSReport("DSP requests...............:   %8d\n", dspReq);
			OSReport("IO requests................:   %8d\n", ioReq);
			OSReport("VI requests................:   %8d\n", viReq);
			OSReport("PE requests................:   %8d\n", peReq);
			OSReport("RF requests................:   %8d\n", rfReq);
			OSReport("FI requests................:   %8d\n", fiReq);
		}
		else
		{
			sprintf( szText, "~w1~al%s: %7d\n", PerfStat[i].text, PerfStat[i].cnt);
			ftext_DebugPrintf( 0.07f, 0.2f + (0.02f * i), szText );
		}
	}
	
	_nCurrStat++;
	if ( _nCurrStat == N_STATS )
	{
		_nCurrStat = 0;
	}
#endif // _GP_SHOW_PERFORMANCE

	return TRUE;
}


//
//
//
static int _FindWindowCallbackFunction( FGCVidWindowCallback_t *pFcnWindowCallback ) 
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
static BOOL _CallWindowCallbackFunctions( FGCVidEvent_e nEvent ) 
{
	BOOL bCallInReverseOrder;
	int i;

	if( nEvent == FGCVID_EVENT_WINDOW_CREATED ) 
	{
		_MasterResFrame = fres_GetFrame();
	} 
	else if ( nEvent == FGCVID_EVENT_WINDOW_DESTROYED ) 
	{
		fres_ReleaseFrame( _MasterResFrame );
	}

	bCallInReverseOrder = (nEvent==FGCVID_EVENT_WINDOW_DESTROYED || nEvent==FGCVID_EVENT_PRE_RESET );

	if( !bCallInReverseOrder ) 
	{
		// Callback in forward order...

		for ( i = 0; i < _nWindowCallbackCount; i++ ) 
		{
			if( !(_ppFcnWindowCallbackTable[i])( nEvent ) ) 
			{
				// Callback returned an error...

				// If that was a window-created event, call the already-visited
				// functions with the window-destroyed event. Otherwise, ignore
				// the return code...
				if ( nEvent == FGCVID_EVENT_WINDOW_CREATED ) 
				{
					fres_ReleaseFrame( _MasterResFrame );

					for ( i--; i >= 0; i-- ) 
					{
						(_ppFcnWindowCallbackTable[i])( FGCVID_EVENT_WINDOW_DESTROYED );
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


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//	GP hang work-around auto-recovery system
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if _GP_HANG_DIAGNOSE
	//
	//	fgcvid_EnableGPHangDiagnose - Sets up the DEMO library to skip past any GP 
	//				hangs andattempt to repair the graphics pipe whenever a timeout
	//				of /timeoutFrames/ occurs.  This will serve as a temporary 
	//				work-around for any GP hangs that may occur.
	//
	void fgcvid_EnableGPHangDiagnose( u32 nTimeoutFrames )
	{
		if ( nTimeoutFrames )
		{
			_nFrameMissThreshold = nTimeoutFrames;
			VISetPreRetraceCallback( _NoHangRetraceCallback );

			// Enable counters for post-hang diagnosis
			_SetGPHangMetric( GX_TRUE );
		}
		else
		{
			_nFrameMissThreshold = 0;
			_SetGPHangMetric( GX_FALSE );
			VISetPreRetraceCallback( NULL );
		}
	}

	//
	//	_NoHangRetraceCallback - VI callback to count missed frames for 
	//				GPHangWorkaround. Also checks for hangs during high 
	//				watermark condition.
	//
	static void _NoHangRetraceCallback( u32 count )
	{
	    #pragma unused (count)
	    static u32 ovFrameCount = 0;
	    static u32 lastOvc = 0;
	    u32        ovc;
	    GXBool     overhi, junk;

	    // Increment the frame counter used by __NoHangDoneRender.

		_nHangFrameCount++;

	    // Check the high watermark status.
	    // If we're still in the same high watermark state as before,
	    // increment the counter and check for time-out.

	    GXGetGPStatus(&overhi, &junk, &junk, &junk, &junk);
	    ovc = GXGetOverflowCount();

	    if (overhi && (ovc == lastOvc))
	    {
	        ovFrameCount++;
	        if ( ovFrameCount >= _nFrameMissThreshold ) 
	        {
	            // We timed out.  Report and diagnose the hang.
	            OSReport("---------WARNING : HANG AT HIGH WATERMARK----------\n");

				_DiagnoseHang();
	        
	            // We cannot easily recover from this situation.  Halt program.
	            OSHalt("Halting program");
	        }
	    }
	    else
	    {
	        lastOvc = ovc;
	        ovFrameCount = 0;
	    }
	}


	//
	//	_SetGPHangMetric - Sets up the GP performance counters in such a way to
	//				enable us to detect the cause of a GP hang.  Note that this takes 
	//				over the performance counters, and you cannot use GXSetGPMetric 
	//				or GXInitXFRasMetric while you have _SetGPHangMetric enabled.
	//
	void _SetGPHangMetric( GXBool enable )
	{
	    if (enable)
	    {
	        // Ensure other counters are off
	        GXSetGPMetric( GX_PERF0_NONE, GX_PERF1_NONE );

	        // Set up RAS Ready counter
	        GXWGFifo.u8  = GX_LOAD_BP_REG;
	        GXWGFifo.u32 = 0x2402c004; // ... 101 10000 00000 00100

	        // Set up SU Ready counter
	        GXWGFifo.u8  = GX_LOAD_BP_REG;
	        GXWGFifo.u32 = 0x23000020; // ... 100 000
	    
	        // Set up XF TOP and BOT busy counters
	        GXWGFifo.u8  = GX_LOAD_XF_REG;
	        GXWGFifo.u16 = 0x0000;
	        GXWGFifo.u16 = 0x1006;
	        GXWGFifo.u32 = 0x00084400; // 10000 10001 00000 00000
	    }
	    else
	    {
	        // Disable RAS counters
	        GXWGFifo.u8  = GX_LOAD_BP_REG;
	        GXWGFifo.u32 = 0x24000000;

	        // Disable SU counters
	        GXWGFifo.u8  = GX_LOAD_BP_REG;
	        GXWGFifo.u32 = 0x23000000;

	        // Disable XF counters
	        GXWGFifo.u8  = GX_LOAD_XF_REG;
	        GXWGFifo.u16 = 0x0000;
	        GXWGFifo.u16 = 0x1006;
	        GXWGFifo.u32 = 0x00000000;
	    }
	}

	//
	//	_DiagnoseHang - Reads performance counters (which should have been set up 
	//				appropriately already) in order to determine why the GP hung.  
	//				The counters must be set as follows:
	//					GXSetGPHangMetric( GX_TRUE );
	//				The above call actually sets up multiple counters, which are 
	//				read using a non-standard method.
	//
	static void _DiagnoseHang( void )
	{
		u32 xfTop0, xfBot0, suRdy0, r0Rdy0;
		u32 xfTop1, xfBot1, suRdy1, r0Rdy1;
		u32 xfTopD, xfBotD, suRdyD, r0RdyD;
		GXBool readIdle, cmdIdle, junk;

		// Read the counters twice in order to see which are changing.
		// This method of reading the counters works in this particular case.
		// You should not use this method to read GPMetric counters.
		GXReadXfRasMetric( &xfBot0, &xfTop0, &r0Rdy0, &suRdy0 );
		GXReadXfRasMetric( &xfBot1, &xfTop1, &r0Rdy1, &suRdy1 );

		// XF Top & Bot counters indicate busy, others indicate ready.
		// Convert readings into indications of who is ready/idle.
		xfTopD = (xfTop1 - xfTop0) == 0;
		xfBotD = (xfBot1 - xfBot0) == 0;
		suRdyD = (suRdy1 - suRdy0) > 0;
		r0RdyD = (r0Rdy1 - r0Rdy0) > 0;

		// Get CP status
		GXGetGPStatus(&junk, &junk, &readIdle, &cmdIdle, &junk);

		OSReport("GP status %d%d%d%d%d%d --> ", readIdle, cmdIdle, xfTopD, xfBotD, suRdyD, r0RdyD);

		// Depending upon which counters are changing, diagnose the hang.
		// This may not be 100% conclusive, but it's what we've observed so far.
		if (!xfBotD && suRdyD)
		{
			OSReport("GP hang due to XF stall bug.\n");
		}
		else if (!xfTopD && xfBotD && suRdyD)
		{
			OSReport("GP hang due to unterminated primitive.\n");
		}
		else if (!cmdIdle && xfTopD && xfBotD && suRdyD)
		{
			OSReport("GP hang due to illegal instruction.\n");
		}
		else if (readIdle && cmdIdle && xfTopD && xfBotD && suRdyD && r0RdyD)
		{
			OSReport("GP appears to be not hung (waiting for input).\n");
		}
		else
		{
			OSReport("GP is in unknown state.\n");
		}
	}
#endif // _GP_HANG_DIAGNOSE