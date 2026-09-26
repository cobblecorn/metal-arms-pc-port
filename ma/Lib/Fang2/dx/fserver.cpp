//////////////////////////////////////////////////////////////////////////////////////
// fserver.cpp - Master file update server for Fang
//
// Author: Nathan Miller
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
// 09/12/02 Miller		Created.
//////////////////////////////////////////////////////////////////////////////////////
#if !defined(_FANGDEF_PRODUCTION_BUILD) && defined(_FANGDEF_PLATFORM_XB)

#include "fang.h"
#include "fdata.h"
#include "fclib.h"
#include "fserver.h"
#include "fmasterfile.h"
#include "ftext.h"
#include "fvid.h"
#include "fxfm.h"
#include "faudio.h"
#include "fpadio.h"
#include "fdraw.h"
#include "floop.h"
#include "fresload.h"
#include "frenderer.h"
#include "xtl.h"
#include "XbDm.h"
#include "fperf.h"
#include <stdio.h>
#include <stdarg.h>

class CCommandQueue
{
public:
	CCommandQueue();

	BOOL IsEmpty(void) {return m_bEmpty;}
	BOOL IsFull(void)  {return !m_bEmpty && (m_uHead == m_uTail);}
	void Clear(void)   {m_uHead = m_uTail = 0; m_bEmpty = TRUE;}

	BOOL AddToQueue(cchar *pszCommand);
	void RemoveFromQueue(char *pszDest, const s32 &sMaxChar);

private:
	enum
	{
		_COMMAND_QUEUE_SIZE = 10
	};

	// If queue size gets larger, use ints
	u8 m_uHead;
	u8 m_uTail;
	BOOL m_bEmpty;
	char m_szQueue[_COMMAND_QUEUE_SIZE][FSERVER_MAX_SEND_BUFFER];
};

#define _STATUS_BUFFER_COUNT	4
#define _STATUS_BUFFER_SIZE		64
#define _SCREEN_SAVER_TIME		(60.0f * 5.0f)			// 5 minutes for screen saver to come on
#define _IDLE_GIVEUP_TIME       (60.0f * 1.0f)          // 1 minutes for server to give up on sync if no data
#define _MASTER_FILE_LOCK		"d:\\master_lock.lck"   // Lock file for master file
#define _USER_LOCK_FILE			"d:\\user_lock.lck"     // User lock file
#define _PROGRESS_UPDATE_RATE	10						// Update ever 10 things

class CFUpdateServer
{
public:
	CFUpdateServer();
	virtual ~CFUpdateServer();

	BOOL ShutdownApplication(void)    {return m_hServerState == _SERVER_STATE_SHUTDOWN_APPLICATION;}
	
	// uLine when 0 is last status line, 1 is second to last, etc and 3 is the latest status message
	cchar *GetStatusLine(cu32 &uLine) {return m_szStatusBuffer[(m_sCurrentStatusLine + uLine) % _STATUS_BUFFER_COUNT];}
	cchar *GetCurrentUserName(void)   {return m_szClientName;}
	BOOL EnableScreenSaver(void)      {return m_fScreenSaverTimer >= _SCREEN_SAVER_TIME;}
	void ResetScreenSaverTimer(void)  {m_fScreenSaverTimer = 0.0f;}

	BOOL Startup(void);
	void Shutdown(void);
	void Reset(void); // Just resets the servers state!
	BOOL Cycle(void);

private:
	enum
	{
		_SERVER_STATE_WAIT_CONNECT = 1,		// Waiting for a connection
		_SERVER_STATE_WAIT_VERSION,			// Waiting for version information
		_SERVER_STATE_SHUTDOWN_APPLICATION, // This is set when we want to take over the Xbox from the current game
		_SERVER_STATE_WAIT_COMMAND,			// Waiting for a command
		_SERVER_STATE_SEND_UPDATE,			// Sending updates to the PC
		_SERVER_STATE_WAIT_UPDATE,			// Waiting for updated file information from PC
		_SERVER_STATE_FILE_PROCESS,			// Processing files from the PC

		_SERVER_STATE_BAD_MASTER,           // State when the master file is bad
	};
	
	void _ResetIdleTimer(void) {m_fIdleTimer = 0.0f;}
	BOOL _IsIdle(void)         {return m_fIdleTimer >= _IDLE_GIVEUP_TIME;}

	void _CommandHandler(const u32 &argc, char *argv[]);
	void _CommandStateHandleConnect(const u32 &argc, char *argv[]);
	void _CommandStateHandleVersion(const u32 &argc, char *argv[]);
	void _CommandStateHandleCommand(const u32 &argc, char *argv[]);
	void _CommandStateHandleEntrySend(const u32 &argc, char *argv[]);
	void _CommandStateHandleUpdate(const u32 &argc, char *argv[]);
	void _CommandStateHandleBadMaster(const u32 &argc, char *argv[]);

	void _SendMasterInformation(void);
	void _SendNextMasterEntry(void);

	void _ProcessFiles(void);
	void _ProcessNextFile(void);
	void _CleanUpTempFiles(void);

	void _CreateTempDirectory(void);

	void _MasterFileLockCreate(void);
	void _MasterFileLockDelete(void);
	BOOL _MasterFileLockExists(void);

	void _UserLockFileLoad(void);
	void _UserLockFileWrite(void);
	void _UserLockFileDelete(void);
	BOOL _UserLockAreUsersSame(cchar *pszUser);
	BOOL _UserLockExists(void);

	void _DropClient(void);

	void _SendNotification(cchar *pszFmt, ...);
	void _SendNotificationBatch(BOOL bSend, cchar *pszFmt, ...);

	static HRESULT __stdcall _CommandProcessor(LPCSTR pszCommand, LPSTR pszResp, DWORD cchResp, PDM_CMDCONT pdmcc);
	
	void _SetStatus(cchar *pszFmt, ...);
	void _SetStatusNoUpdate(cchar *pszFmt, ...);

	static CFUpdateServer *m_pServer;				// Pointer to the server object
	static CRITICAL_SECTION m_hCriticalSection;
	BOOL m_bCriticalSectionSetup;

	CCommandQueue m_CommandQueue;					
	char szLocalBuff[FSERVER_MAX_SEND_BUFFER];		// Holds commands sent to the server
	char szSendBuffer[FSERVER_MAX_SEND_BUFFER];     // Holds commands server is sending

	BOOL m_bRun;									// Should the server run
	u8 m_hServerState;								// State we are in

	u32 m_uNumEntriesSent;							// Number of master file entries sent to 

	u32 m_uNumExpectedUpdates;						// Expected number of updates from PC
	u32 m_uNumReceivedUpdates;						// Updates we have so far
	FDataPrjFile_Entry_t *m_pReceivedEntries;		// Stuff entries we got in here
	
	u32 m_uNumFilesProcessed;						// Files on disk we have processed

	char m_szClientName[FSERVER_MAX_CLIENT_NAME];	// Connected client's name

	BOOL m_bBadMasterFile;
	CFMasterFile m_MasterFile;

	// Status line stuff
	BOOL m_bHaveLine;
	u32 m_sCurrentStatusLine;
	char m_szStatusBuffer[_STATUS_BUFFER_COUNT][_STATUS_BUFFER_SIZE];

	f32 m_fScreenSaverTimer;
	f32 m_fIdleTimer;

	BOOL m_bHasUserLock;							// True if we have a lock
	char m_szUserLockName[FSERVER_MAX_CLIENT_NAME]; // Owner of the lock
};

static BOOL _bModuleInitialized = FALSE;
static BOOL _bServerTakeover = FALSE;
static BOOL _bGraphicsStarted = FALSE;

static CFUpdateServer _server;

static CFXfm _ViewportFX;
static FDrawVtx_t _LogoVerts[4];
static FDrawVtx_t _BackgroundVerts[5];
static CFTexInst _TextureInst;

BOOL fserver_ModuleStartup( void ) {
	FASSERT( !_bModuleInitialized );
	_bModuleInitialized = TRUE;

	return _server.Startup();
}

void fserver_ModuleShutdown( void ) {
	FASSERT( _bModuleInitialized );
	_bModuleInitialized = FALSE;

	_server.Shutdown();
}

BOOL fserver_GetTakeover( void ) {
	return _bServerTakeover;
}

void fserver_SetTakeover( BOOL bTake ) {
	_bServerTakeover = bTake;

	if (_bServerTakeover)
		_server.ResetScreenSaverTimer();
}

void fserver_Reset( void ) {
	_server.Reset();
}

BOOL fserver_Work( void ) {
	BOOL bRes = _server.Cycle();

	// No graphics, we are done
	if (!_bGraphicsStarted)
		return bRes;

	FPerf_Display_Type_e nPriorSetting = FPerf_nDisplayPerfType;
	FPerf_nDisplayPerfType = FPERF_TYPE_NONE;

	fvid_Begin();
	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );

	fviewport_SetActive(FViewport_pDefaultOrtho);
	_ViewportFX.InitStackWithView();
	
	/*frenderer_Push(FRENDERER_DRAW, NULL);
	fdraw_Depth_EnableWriting(FALSE);
	fdraw_Depth_SetTest(FDRAW_DEPTHTEST_ALWAYS);
	fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE);
	fdraw_Color_SetFunc(FDRAW_COLORFUNC_DIFFUSETEX_AIAT);
	fdraw_SetTexture(&_TextureInst);
	fdraw_PrimList(FDRAW_PRIMTYPE_TRISTRIP, _LogoVerts, 4);
	frenderer_Pop();*/

	if (_server.EnableScreenSaver())
	{
		// Screen saver time
		fviewport_SetActive(FViewport_pDefaultOrtho);
		_ViewportFX.InitStackWithView();

		frenderer_Push(FRENDERER_DRAW, NULL);
		fdraw_Depth_EnableWriting(FALSE);
		fdraw_Depth_SetTest(FDRAW_DEPTHTEST_ALWAYS);
		fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_ALPHA_TIMES_DST);
		fdraw_Color_SetFunc(FDRAW_DEFAULT_COLORFUNC);
		fdraw_SetTexture(NULL);
		fdraw_PrimList(FDRAW_PRIMTYPE_TRISTRIP, _BackgroundVerts, 4);
		frenderer_Pop();
	}
	else
	{
		cchar *pszCurrUser = _server.GetCurrentUserName();

		if (pszCurrUser[0] != 0)
			ftext_Printf(0.5f, 0.1f, "~w1~ac~c90000099~S1.30Server Being Used By:\n~B9~c90909099%s", pszCurrUser);
		else
			ftext_Printf(0.5f, 0.1f, "~w1~ac~c00005599~S1.30Server Not In Use");

		ftext_Printf(0.5f, 0.59f, "~w1~ac~c00900050~S0.85%s", _server.GetStatusLine(0));
		ftext_Printf(0.5f, 0.62f, "~w1~ac~c00900060~S0.85%s", _server.GetStatusLine(1));
		ftext_Printf(0.5f, 0.65f, "~w1~ac~c00900070~S0.85%s", _server.GetStatusLine(2));
		ftext_Printf(0.5f, 0.68f, "~w1~ac~c00900099~S0.85%s", _server.GetStatusLine(3));

		ftext_Printf( 0.68f, 0.05f, "~C92929299~w1~al~s0.69Ver %d.%d.%d", fversion_GetToolMajorVer(), 
																		 fversion_GetToolMinorVer(),
																		 fversion_GetToolSubVer() );
	}

	fvid_End();
	fvid_Swap();

	FPerf_nDisplayPerfType = nPriorSetting;

	return bRes;
}

BOOL fserver_ShutdownApplication( void ) {
	return _server.ShutdownApplication();	
}

BOOL fserver_GraphicsStartup( void ) {
	if (_bGraphicsStarted)
	{
		return TRUE;
	}

	FVidWin_t vidInfo;
	
	vidInfo.VidDev.nFlags = FVID_DEVFLAG_HW_TNL;
	vidInfo.VidDev.nOrdinal = 0;
	vidInfo.VidDev.nRenderer = FVID_RENDERER_HARDWARE;
	fclib_strcpy(vidInfo.VidDev.szName, "Xbox NV2A");
	vidInfo.VidMode.nColorBits = 32;
	vidInfo.VidMode.nDepthBits = 24;
	vidInfo.VidMode.nStencilBits = 8;
	vidInfo.VidMode.nPixelsAcross = 640;
	vidInfo.VidMode.nPixelsDown = 480;
	vidInfo.VidMode.nFlags = FVID_MODEFLAG_NONE;
	vidInfo.fUnitFSAA = 0.0f;
	vidInfo.nSwapInterval = 0;

	if (!fvid_CreateWindow(&vidInfo))
	{
		DEVPRINTF("fvid_CreateWindow() failed\n");
		return FALSE;
	}

/*
	FPadio_Init_t oPadioInit;
	fang_MemZero(&oPadioInit, sizeof(oPadioInit));

	if (FPADIO_NO_ERROR != fpadio_Install(&oPadioInit))
	{
		DEVPRINTF("fpadio_Install() failed\n");
		fvid_DestroyWindow();
		return FALSE;
	}
*/
	if (ftext_Install() != FTEXT_NO_ERROR)
	{
		DEVPRINTF("ftext_Install() failed\n");
		fvid_DestroyWindow();
		return FALSE;
	}

	_ViewportFX.Identity();

	fang_MemZero(_LogoVerts, sizeof(_LogoVerts));

	_LogoVerts[0].ColorRGBA.Set(1.0f, 1.0f, 1.0f, 1.0f);
	_LogoVerts[1].ColorRGBA.Set(1.0f, 1.0f, 1.0f, 1.0f);
	_LogoVerts[2].ColorRGBA.Set(1.0f, 1.0f, 1.0f, 1.0f);
	_LogoVerts[3].ColorRGBA.Set(1.0f, 1.0f, 1.0f, 1.0f);

	f32 fX = FViewport_pDefaultOrtho->HalfRes.x;
	f32 fY = FViewport_pDefaultOrtho->HalfRes.y;

	_LogoVerts[0].Pos_MS.Set(fX - 128.0f, fY - 128.0f, 1.0f);
	_LogoVerts[1].Pos_MS.Set(fX + 128.0f, fY - 128.0f, 1.0f);
	_LogoVerts[2].Pos_MS.Set(fX - 128.0f, fY + 128.0f, 1.0f);
	_LogoVerts[3].Pos_MS.Set(fX + 128.0f, fY + 128.0f, 1.0f);

	_LogoVerts[0].ST.Set(0.0f, 0.0f);
	_LogoVerts[1].ST.Set(1.0f, 0.0f);
	_LogoVerts[2].ST.Set(0.0f, 1.0f);
	_LogoVerts[3].ST.Set(1.0f, 1.0f);

	_BackgroundVerts[0].ColorRGBA.Set(0.0f, 0.1f);
	_BackgroundVerts[1].ColorRGBA.Set(0.0f, 0.1f);
	_BackgroundVerts[2].ColorRGBA.Set(0.0f, 0.1f);
	_BackgroundVerts[3].ColorRGBA.Set(0.0f, 0.1f);

	_BackgroundVerts[0].Pos_MS.Set(0.0f, 0.0f, 1.0f);
	_BackgroundVerts[1].Pos_MS.Set(FViewport_pDefaultOrtho->Res.x, 0.0f, 1.0f);
	_BackgroundVerts[2].Pos_MS.Set(0.0f, FViewport_pDefaultOrtho->Res.y, 1.0f);
	_BackgroundVerts[3].Pos_MS.Set(FViewport_pDefaultOrtho->Res.x, FViewport_pDefaultOrtho->Res.y, 1.0f);

	_TextureInst.SetTexDef((FTexDef_t *) fresload_Load(FTEX_RESNAME, "happyturtl"));

	_bGraphicsStarted = TRUE;

	return TRUE;
}

// ////////////////////
// CCommandQueue class
// ////////////////////
CCommandQueue::CCommandQueue()
{
	m_uHead = 0;
	m_uTail = 0;
	m_bEmpty = TRUE;
}

BOOL CCommandQueue::AddToQueue(cchar *pszCommand)
{
	FASSERT(pszCommand);
	FASSERT(fclib_strlen(pszCommand) > 0);

	if (fclib_strlen(pszCommand) >= FSERVER_MAX_SEND_BUFFER)
	{
		return FALSE;
	}

	// queue is full
	if (IsFull())
	{
		return FALSE;
	}

	if (m_bEmpty)
	{
		m_bEmpty = FALSE;
	}

	fclib_strcpy(m_szQueue[m_uTail], pszCommand);

	m_uTail = (m_uTail + 1) % _COMMAND_QUEUE_SIZE;

	return TRUE;
}

void CCommandQueue::RemoveFromQueue(char *pszDest, const s32 &sMaxChar)
{
	FASSERT(pszDest);
	FASSERT(sMaxChar > 0);

	if (IsEmpty())
	{
		return;
	}

	if (fclib_strlen(m_szQueue[m_uHead]) >= sMaxChar)
	{
		pszDest[0] = 0;
		return;
	}

	fclib_strcpy(pszDest, m_szQueue[m_uHead]);

	m_uHead = (m_uHead + 1) % _COMMAND_QUEUE_SIZE;

	if (m_uHead == m_uTail)
	{
		m_bEmpty = TRUE;
	}
}

// !! Uses a | to parse between elements, not a space.  Taken from Xbox SDK samples
// | is used since some files names in the master file contain spaces
static u32 _CmdToArgv(char *szCmd, char *szArg[], const u32 &maxargs)
{
    u32 argc = 0;
    u32 argcT = 0;
    char *szNil = szCmd + fclib_strlen(szCmd);

    while(argcT < maxargs)
    {
        // eat whitespace
        while(*szCmd && (*szCmd == ' '))
            szCmd++;

        if(!*szCmd)
        {
            szArg[argcT++] = (char *)szNil;
        }
        else
        {
            // find the end of this arg
            char chEnd = (*szCmd == '"' || *szCmd == '\'') ? *szCmd++ : '|';
            char *szArgEnd = szCmd;
            while(*szArgEnd && (*szArgEnd != chEnd))
                szArgEnd++;

            // record this bad boy
            szArg[argcT++] = szCmd;
            argc = argcT;

            // move szArg to the next argument (or not)
            szCmd = *szArgEnd ? szArgEnd + 1 : szArgEnd;
            *szArgEnd = 0;
        }
    }

    return argc;
}

// ////////////////////
// CFUpdateServer class
// ////////////////////
CRITICAL_SECTION CFUpdateServer::m_hCriticalSection;
CFUpdateServer *CFUpdateServer::m_pServer = 0;

CFUpdateServer::CFUpdateServer()
{
	FASSERT(!m_pServer);

	m_bCriticalSectionSetup = FALSE;

	m_szClientName[0] = 0;

	m_uNumFilesProcessed = 0;
	m_uNumExpectedUpdates = 0;
	m_uNumReceivedUpdates = 0;
	m_pReceivedEntries = 0;

	m_uNumEntriesSent = 0;

	m_hServerState = _SERVER_STATE_WAIT_CONNECT;

	m_pServer = this;
	m_bRun = FALSE;

	m_sCurrentStatusLine = 0;
	m_bHaveLine = FALSE;

	for (u32 uCnt = 0; uCnt < _STATUS_BUFFER_COUNT; ++uCnt)
	{
		m_szStatusBuffer[uCnt][0] = 0;
	}

	m_bHasUserLock = FALSE;
	m_szUserLockName[0] = FALSE;

	ResetScreenSaverTimer();
	m_fIdleTimer = 0.0f;

	m_bBadMasterFile = FALSE;
}

CFUpdateServer::~CFUpdateServer()
{
	if (m_bCriticalSectionSetup)
	{
		Shutdown();
	}

	delete [] m_pReceivedEntries;
}

BOOL CFUpdateServer::Startup(void)
{
	_CreateTempDirectory();

	_SetStatus("Server startup...");

	InitializeCriticalSection(&m_hCriticalSection);
	m_bCriticalSectionSetup = TRUE;

	//DmUseSharedConnection(TRUE);

    // Register our command handler with the debug monitor
    HRESULT hr = DmRegisterCommandProcessor(FSERVER_COMMAND_PREFIX, _CommandProcessor);

	if (FAILED(hr))
	{
		DEVPRINTF("fserver - DmRegisterCommandProcessor() failed!\n");
		return 0;
	}

	_CleanUpTempFiles();

	if (_MasterFileLockExists())
	{
		DEVPRINTF("fserver - WARNING!!!  MASTER FILE LOCK EXISTS!  MASTER FILE MAY BE CORRUPT!\n");
		_SetStatus("~C90000099Master file lock exists!");
		_SetStatus("~C90000099Deleting master file...");
		
		m_bBadMasterFile = TRUE;
	}

	_UserLockFileLoad();

	_SetStatus("Server started up!");

	if (m_bHasUserLock)
	{
		_SetStatus("%s has server locked!", m_szUserLockName);
	}

	m_bRun = TRUE;
	
	ResetScreenSaverTimer();
	_ResetIdleTimer();

	return TRUE;
}

void CFUpdateServer::Shutdown(void)
{
	Reset();
	_CleanUpTempFiles();

	DeleteCriticalSection(&m_hCriticalSection);
	m_bCriticalSectionSetup = FALSE;

	m_bRun = FALSE;

	m_MasterFile.Reset();

	_SetStatus("Shutdown...");
}

// Resets the servers state, but still lets the server run
void CFUpdateServer::Reset(void)
{
	delete [] m_pReceivedEntries;
	m_pReceivedEntries = 0;

	m_szClientName[0] = 0;

	m_uNumFilesProcessed = 0;
	m_uNumExpectedUpdates = 0;
	m_uNumReceivedUpdates = 0;
	m_pReceivedEntries = 0;

	m_uNumEntriesSent = 0;

	m_hServerState = _SERVER_STATE_WAIT_CONNECT;

	m_bBadMasterFile = FALSE;
	
	// Don't reset the master file here
	//m_MasterFile.Reset();

	ResetScreenSaverTimer();
	_ResetIdleTimer();
	
	EnterCriticalSection(&m_hCriticalSection);
	m_CommandQueue.Clear();
	LeaveCriticalSection(&m_hCriticalSection);
}

BOOL CFUpdateServer::Cycle(void)
{
	if (!m_bRun)
	{
		return FALSE;
	}

	m_fScreenSaverTimer += FLoop_fPreviousLoopSecs;
	m_fIdleTimer += FLoop_fPreviousLoopSecs;

	u32 argc;
	char *pArgv[FSERVER_MAX_COMMAND_ARGUMENTS];

	// Load the master file the cycle after we are put into the shutdown state
	if (m_hServerState == _SERVER_STATE_SHUTDOWN_APPLICATION)
	{
		if (!m_MasterFile.IsFileLoaded() && !m_MasterFile.Load(FSERVER_MASTER_FILE_NAME))
		{
			m_hServerState = _SERVER_STATE_BAD_MASTER;
			m_bBadMasterFile = TRUE;

			_SendNotification("master_send");
		}
		else
		{
			//m_hServerState = _SERVER_STATE_WAIT_COMMAND;
			_SendNotification("version");
			m_hServerState = _SERVER_STATE_WAIT_VERSION;
		}

		// Reset timer here since we could be showing graphics for the first time
		ResetScreenSaverTimer();
	}

	// Process all commands that we have
	while (!m_CommandQueue.IsEmpty())
	{
		EnterCriticalSection(&m_hCriticalSection);
		m_CommandQueue.RemoveFromQueue(szLocalBuff, FSERVER_MAX_SEND_BUFFER);
		LeaveCriticalSection(&m_hCriticalSection);

		argc = _CmdToArgv(szLocalBuff, pArgv, FSERVER_MAX_COMMAND_ARGUMENTS);
		_CommandHandler(argc, pArgv);
		argc = 0;

		// Once we hit shutdown, don't process any more commands
		if (m_hServerState == _SERVER_STATE_SHUTDOWN_APPLICATION)
		{
			break;
		}
	}

	if (m_hServerState == _SERVER_STATE_FILE_PROCESS)
	{
		_ProcessNextFile();
	}

	if (m_hServerState != _SERVER_STATE_BAD_MASTER && 
		m_hServerState != _SERVER_STATE_FILE_PROCESS &&
		m_hServerState != _SERVER_STATE_WAIT_CONNECT && _IsIdle())
	{
		_SetStatus("~C90000099Client idle during sync process!");
		_SetStatus("~C90000099Booting current client!");

		Reset();
	}

	return m_bRun;
}

void CFUpdateServer::_CommandHandler(const u32 &argc, char *argv[])
{
	if (argc <= 0)
	{
		return;
	}

	// We got some sort of command, not idle
	ResetScreenSaverTimer();
	_ResetIdleTimer();

	char *pszCmd = argv[0];

	// This is here so we can disconnect at any time, no matter what the state
	if (!fclib_strncmp("disconnect", pszCmd, 10))
	{
		if (m_szClientName[0])
		{
			_SetStatus("%s has disconnected.", m_szClientName);
		}

		_DropClient();

		return;
	}

	switch (m_hServerState)
	{
		case _SERVER_STATE_WAIT_CONNECT:
			_CommandStateHandleConnect(argc, argv);
		break;
		case _SERVER_STATE_WAIT_VERSION:
			_CommandStateHandleVersion(argc, argv);
		break;
		case _SERVER_STATE_WAIT_COMMAND:
			_CommandStateHandleCommand(argc, argv);
		break;
		case _SERVER_STATE_SEND_UPDATE:
			_CommandStateHandleEntrySend(argc, argv);
		break;
		case _SERVER_STATE_WAIT_UPDATE:
			_CommandStateHandleUpdate(argc, argv);
		break;
		case _SERVER_STATE_BAD_MASTER:
			_CommandStateHandleBadMaster(argc, argv);
		break;
	}
}

void CFUpdateServer::_CommandStateHandleConnect(const u32 &argc, char *argv[])
{
	char *pszCmd = argv[0];

	// connect - Connection attempt by the client
	// boot    - Kick the current client, but in this case, just delete the lock file

	if (!fclib_strncmp("connect", pszCmd, 7))
	{
		if (argc == 2)
		{
			fclib_strncpy(m_szClientName, argv[1], FSERVER_MAX_CLIENT_NAME);
		}
		else
		{
			fclib_strcpy(m_szClientName, "Unknown");
		}

		_SetStatus("%s has connected.", m_szClientName);

		// Newly connected user has the lock
		fclib_strcpy(m_szUserLockName, m_szClientName);

		_UserLockFileWrite();

		// server application is taking over
		fserver_SetTakeover(TRUE);

		// Don't do this since we may have to send master file
		if (!m_bBadMasterFile)
		{
			m_hServerState = _SERVER_STATE_SHUTDOWN_APPLICATION;
		}
		else
		{
			m_hServerState = _SERVER_STATE_BAD_MASTER;
		}
	}
	// You can boot in the function below as well
	else if (!fclib_strncmp("boot", pszCmd, 4))
	{
		if (m_szClientName[0])
		{
			_SetStatus("Booting %s from server.", m_szClientName);
			_SendNotification("disconnect");
		}
		_DropClient();
		
		// Delete the lock and reset the lock info
		_UserLockFileDelete();
	}
}

void CFUpdateServer::_CommandStateHandleVersion(const u32 &argc, char *argv[])
{
	if (argc != 10)
	{
		_SetStatus("Bad version command!");
		goto _Error;
	}

	if (!fclib_strncmp("version", argv[0], 7))
	{

		if (m_MasterFile.GetTgaVersionFromHeader() != (u32) atoi(argv[1]))
			goto _Error;

		if (m_MasterFile.GetApeVersionFromHeader() != (u32) atoi(argv[2]))
			goto _Error;

		if (m_MasterFile.GetMtxVersionFromHeader() != (u32) atoi(argv[3]))
			goto _Error;

		if (m_MasterFile.GetCsvVersionFromHeader() != (u32) atoi(argv[4]))
			goto _Error;

		if (m_MasterFile.GetFntVersionFromHeader() != (u32) atoi(argv[5]))
			goto _Error;

		if (m_MasterFile.GetSmaVersionFromHeader() != (u32) atoi(argv[6]))
			goto _Error;

		if (m_MasterFile.GetGtVersionFromHeader() != (u32) atoi(argv[7]))
			goto _Error;

		if (m_MasterFile.GetWvbVersionFromHeader() != (u32) atoi(argv[8]))
			goto _Error;

		if (m_MasterFile.GetFprVersionFromHeader() != (u32) atoi(argv[9]))
			goto _Error;

		_SendNotification("ok_version");

		m_hServerState = _SERVER_STATE_WAIT_COMMAND;
	}

	return;

_Error:
	_SetStatus("Master file versions do not match");
	_SendNotification("master_send");
	m_hServerState = _SERVER_STATE_BAD_MASTER;
	m_bBadMasterFile = TRUE;
}

void CFUpdateServer::_CommandStateHandleCommand(const u32 &argc, char *argv[])
{
	char *pszCmd = argv[0];

	// shutdown        - Shutdown the server
	// get_master_info - Request from client for master file information on Xbox
	// update          - Tells the server to get ready for updated file information from PC
	//					 Has format of "update|<number>" where <number> is the number of files to update
	// boot			   - Kick the current client, but in this case, just delete the lock file


	if (!fclib_strncmp("shutdown", pszCmd, 8))
	{
		m_bRun = FALSE;
	}
	else if (!fclib_strncmp("get_master_info", pszCmd, 15))
	{
		_SendMasterInformation();
	}
	else if (!fclib_strncmp("update", pszCmd, 6))
	{
		if (argc < 2)
		{
			return;
		}

		m_uNumExpectedUpdates = (u32) atoi(argv[1]);
		m_uNumReceivedUpdates = 0;

		if (m_pReceivedEntries)
		{
			_SetStatus("Already in the process of an update\n");

			delete [] m_pReceivedEntries;
			return;
		}

		m_pReceivedEntries = new FDataPrjFile_Entry_t[m_uNumExpectedUpdates];

		if (!m_pReceivedEntries)
		{
			_DropClient();
			return;
		}

		m_hServerState = _SERVER_STATE_WAIT_UPDATE;

		_SetStatus("Expected file updates %d.", m_uNumExpectedUpdates);
	} 
	// You can boot in the above function as well
	else if (!fclib_strncmp("boot", pszCmd, 4))
	{
		if (m_szClientName[0])
		{
			_SetStatus("Booting %s from server.", m_szClientName);
			_SendNotification("disconnect");
		}
		_DropClient();
		
		// Delete the lock and reset the lock info
		_UserLockFileDelete();
	}
	else if (!fclib_strncmp("master_send", pszCmd, 11))
	{
		_CommandStateHandleBadMaster(argc, argv);
	}
}

void CFUpdateServer::_CommandStateHandleEntrySend(const u32 &argc, char *argv[])
{
	// ok - Got a file update from the client, tell them we got it so client can send next

	if (!fclib_strncmp("ok", argv[0], 2))
	{
		_SendNextMasterEntry();	
	}
}

void CFUpdateServer::_CommandStateHandleUpdate(const u32 &argc, char *argv[])
{
	char *pszCmd = argv[0];

	// NKM - 01/04/03 - Added CRC
	//
	// update_file - A file update from the PC.  Has format of "update_file|<name>|<modified time>|<CRC>"
	//				 where <name> is a file name and <modified time> and <CRC> are unsigned integers in string form.
	// update_done - Message from the client to tell the server the file update send is done

	if (!fclib_strncmp("update_file", pszCmd, 11))
	{
		if (argc != 4)
		{
			return;
		}

		fclib_strcpy(m_pReceivedEntries[m_uNumReceivedUpdates].szFilename, argv[1]);
		m_pReceivedEntries[m_uNumReceivedUpdates].nModifiedTime = (u32) atoi(argv[2]);
		m_pReceivedEntries[m_uNumReceivedUpdates].nCRC = (u32) atoi(argv[3]);

		++m_uNumReceivedUpdates;

		if (!(m_uNumReceivedUpdates % _PROGRESS_UPDATE_RATE))
		{
			_SetStatusNoUpdate("Got %d%% of file entries...", (u32) (100.0f * ((f32) m_uNumReceivedUpdates / m_uNumExpectedUpdates)));
		}

		// Don't send okay for last entry
		if (m_uNumReceivedUpdates < m_uNumExpectedUpdates)
		{
			_SendNotification("ok");
		}

		if (m_uNumReceivedUpdates == m_uNumExpectedUpdates)
		{
			// Set this since we may not display 100%
			_SetStatusNoUpdate("Got 100%% of file entries...");
			_SetStatus("Got all file updates from PC!");
		}
	}
	else if (!fclib_strncmp("update_done", pszCmd, 11))
	{
		_SetStatus("File update done!");
		_ProcessFiles();
	}
}

void CFUpdateServer::_CommandStateHandleBadMaster(const u32 &argc, char *argv[])
{
	char *pszCmd = argv[0];

	// master_send - The master file is going to be sent
	//			
	// master_end - We now have a good master file

	if (!fclib_strncmp("master_send", pszCmd, 11))
	{
		// Set the server state since this method can be called from _CommandStateHandleCommand()
		// This state is set since it may not be set on client connect, it may come from the client
		// wanting to send the master file over since it may take too long to update it.
		m_hServerState = _SERVER_STATE_BAD_MASTER;
		m_bBadMasterFile = TRUE;

		m_MasterFile.Close();
		m_MasterFile.Reset();

		_SetStatus("Waiting for master file...");
		
		DeleteFile(FSERVER_MASTER_FILE_NAME);

		_MasterFileLockCreate();
	}
	else if (!fclib_strncmp("master_end", pszCmd, 10))
	{
		_SetStatus("Got master file!");

		//_SendNotification("disconnect");
		//_DropClient();

		m_hServerState = _SERVER_STATE_SHUTDOWN_APPLICATION;
		m_bBadMasterFile = FALSE;
		_MasterFileLockDelete();
	}
}

void CFUpdateServer::_SendMasterInformation(void)
{
	_SetStatus("Sending %d file entries...", m_MasterFile.GetNumFileEntries());

	m_hServerState = _SERVER_STATE_SEND_UPDATE;
	m_uNumEntriesSent = 0;

	// Tell client we are about to start the send of the master file information
	_SendNotification("num_entries|%d", m_MasterFile.GetNumFileEntries());
	// Send the first entry
	_SendNextMasterEntry();
}

void CFUpdateServer::_SendNextMasterEntry(void)
{
	u32 uNum = m_MasterFile.GetNumFileEntries();
	BOOL bSend = FALSE;

	for (u32 uCnt = 0; uCnt < FSERVER_MAX_COMMANDS; ++uCnt)
	{
		const FDataPrjFile_Entry_t *pEntry = m_MasterFile.GetFileEntry(m_uNumEntriesSent);

		// Send the next entry in the list
		// NKM - 01/04/03 - Added CRC
		_SendNotificationBatch(FALSE, "entry|%d|%s|%d|%d", m_uNumEntriesSent, pEntry->szFilename, pEntry->nModifiedTime, pEntry->nCRC);

		++m_uNumEntriesSent;

		if (!(m_uNumEntriesSent % _PROGRESS_UPDATE_RATE))
		{
			_SetStatusNoUpdate("Sent %d%% of file entries...", (u32) (((f32) m_uNumEntriesSent / uNum) * 100.0f));
		}

		if (m_uNumEntriesSent == uNum)
		{
			// Make sure we send out anything we have
			_SendNotificationBatch(TRUE, "");

			// We are done
			_SendNotification("get_master_info_done");

			// Set this since we may not display 100%
			_SetStatusNoUpdate("Sent 100%% of file entries...");
			_SetStatus("Done sending file entries.");
			_SetStatus("Waiting for update information...");

			m_uNumEntriesSent = 0;
			m_hServerState = _SERVER_STATE_WAIT_COMMAND;

			return;
		}
	}

	// Send out the junk we just buffered
	_SendNotificationBatch(TRUE, "");
}

void CFUpdateServer::_ProcessFiles(void)
{
	m_uNumFilesProcessed = 0;
	m_hServerState = _SERVER_STATE_FILE_PROCESS;

	_SetStatus("Processing files...");
	_MasterFileLockCreate();
	_ProcessNextFile();
}

void CFUpdateServer::_ProcessNextFile(void)
{
	char szPath[MAX_PATH];
	u32 uCnt = m_uNumFilesProcessed;

	// This assumes that the file has been sent to the Xbox already and are in the temporary directory.
	fclib_strcpy(szPath, FSERVER_SERVER_TEMP_DIR);
	fclib_strcat(szPath, m_pReceivedEntries[uCnt].szFilename);

	if (!m_MasterFile.UpdateFile(m_pReceivedEntries[uCnt].szFilename, szPath, m_pReceivedEntries[uCnt].nModifiedTime, m_pReceivedEntries[uCnt].nCRC))
	{
		_SetStatus("Failed to process \"%s\"!", m_pReceivedEntries[uCnt].szFilename);
		_SendNotification("done|%d", uCnt);
		_DropClient();

		return;
	}
		
	// Not idle, we are working
	ResetScreenSaverTimer();

	++m_uNumFilesProcessed;

	// Send a work notification if needed
	if (!(m_uNumFilesProcessed % FSERVER_FILE_SEND_WORK_MSG))
	{
		_SendNotification("work");
	}

	if (!(m_uNumFilesProcessed % _PROGRESS_UPDATE_RATE))
	{
		_SetStatusNoUpdate("Processing files - %d%%", 
			(u32) (100.0f * ((f32) m_uNumFilesProcessed / m_uNumReceivedUpdates)));
	}

	if (m_uNumFilesProcessed == m_uNumReceivedUpdates)
	{
		_MasterFileLockDelete();
		_SendNotification("done|%d", m_uNumReceivedUpdates);

		m_hServerState = _SERVER_STATE_WAIT_COMMAND;

		m_uNumFilesProcessed = 0;
		m_uNumReceivedUpdates = 0;
		m_uNumExpectedUpdates = 0;

		delete [] m_pReceivedEntries;
		m_pReceivedEntries = 0;

		// Set this since we may not display 100%
		_SetStatusNoUpdate("Processing files - 100%%");

		_CleanUpTempFiles();
	
		_SetStatus("Done updating master file!");
	}
}

void CFUpdateServer::_CleanUpTempFiles(void)
{
	char szPath[MAX_PATH];

	fclib_strcpy(szPath, FSERVER_SERVER_TEMP_DIR);
	fclib_strcat(szPath, "*");

    WIN32_FIND_DATA wfd;
    HANDLE hFind;
    
    // Start the find and check for failure.
    hFind = FindFirstFile(szPath, &wfd);

    if (INVALID_HANDLE_VALUE == hFind)
	{
		return;
	}

	_SetStatus("Cleaning up...");

	do
	{
		fclib_strcpy(szPath, FSERVER_SERVER_TEMP_DIR);
		fclib_strcat(szPath, wfd.cFileName);

		if (!DeleteFile(szPath))
		{
			DEVPRINTF("Failed to delete \"%s\"!", wfd.cFileName);
		}

	} while (FindNextFile(hFind, &wfd));

    FindClose(hFind);
}

void CFUpdateServer::_CreateTempDirectory(void)
{
	DWORD attrib = GetFileAttributes("D:\\temp\\");

	if (attrib != FILE_ATTRIBUTE_DIRECTORY)
	{
		CreateDirectory("D:\\temp\\", NULL);
	}
}

void CFUpdateServer::_MasterFileLockCreate(void)
{
	FILE *pFile = fopen(_MASTER_FILE_LOCK, "w");

	if (!pFile)
		return;

	fclose(pFile);
}

void CFUpdateServer::_MasterFileLockDelete(void)
{
	DeleteFile(_MASTER_FILE_LOCK);
}

BOOL CFUpdateServer::_MasterFileLockExists(void)
{
	FILE *pFile = fopen(_MASTER_FILE_LOCK, "r");

	if (pFile)
	{
		fclose(pFile);
		return TRUE;
	}

	return FALSE;
}

void CFUpdateServer::_UserLockFileLoad(void)
{
	FFileHandle hFile = ffile_Open(_USER_LOCK_FILE, FFILE_OPEN_RONLY, TRUE);

	if (!FFILE_IS_VALID_HANDLE(hFile))
	{
		return;
	}

	ffile_Scanf(hFile, "%s\n", m_szUserLockName);

	// if white space at start, assume it is a bad name
	if (fclib_TrimLeadingWhiteSpace(m_szUserLockName) != m_szUserLockName)
	{
		m_szUserLockName[0] = 0;
	}

	if (m_szUserLockName[0])
	{
		m_bHasUserLock = TRUE;
	}

	ffile_Close(hFile);
}

void CFUpdateServer::_UserLockFileWrite(void)
{
	if (!m_szUserLockName[0])
	{
		return;
	}

	FFileHandle hFile = ffile_Open(_USER_LOCK_FILE, FFILE_OPEN_TRUNC_OR_CREATE_WONLY, TRUE);

	if (!FFILE_IS_VALID_HANDLE(hFile))
	{
		return;
	}

	ffile_Write(hFile, fclib_strlen(m_szUserLockName), m_szUserLockName);
	ffile_Write(hFile, 1, "\n");

	ffile_Close(hFile);
}

void CFUpdateServer::_UserLockFileDelete(void)
{
	DeleteFile(_USER_LOCK_FILE);

	m_szUserLockName[0] = 0;
	m_bHasUserLock = FALSE;
}

BOOL CFUpdateServer::_UserLockAreUsersSame(cchar *pszUser)
{
	return !fclib_strcmp(pszUser, m_szUserLockName);
}

BOOL CFUpdateServer::_UserLockExists(void)
{
	return m_szUserLockName[0] != 0;
}

void CFUpdateServer::_DropClient(void)
{
	if (!m_szClientName[0])
		return;

	Reset();
	_CleanUpTempFiles();
}

void CFUpdateServer::_SendNotification(cchar *pszFmt, ...)
{
    va_list arglist;

	fclib_strncpy(szSendBuffer, FSERVER_COMMAND_PREFIX, FSERVER_COMMAND_PREFIX_LEN);
	szSendBuffer[FSERVER_COMMAND_PREFIX_LEN] = '!';

    // format arguments
    va_start(arglist, pszFmt);

    _vsnprintf(szSendBuffer + FSERVER_COMMAND_PREFIX_LEN + 1,
		FSERVER_MAX_SEND_BUFFER - FSERVER_COMMAND_PREFIX_LEN - 1, pszFmt, arglist);

    va_end(arglist);

    HRESULT hr = DmSendNotificationString(szSendBuffer);

	szSendBuffer[0] = 0;

	// If we fail, drop the client
	if (FAILED(hr))
	{
		_DropClient();
		_SetStatus("Failed to send notification to client.");
	}
}

// This allows you to batch notification messages
// After the first message is put into the buffer, a FSERVER_COMMAND_TOKEN will be appended
// to it after each call of this method.  When the method is called with bSend equal to TRUE,
// the entire buffer will be sent.  
//
// NOTE: When bSend is true, no formatting will be done on the params passed to the method.
void CFUpdateServer::_SendNotificationBatch(BOOL bSend, cchar *pszFmt, ...)
{
	if (bSend)
	{
		HRESULT hr = DmSendNotificationString(szSendBuffer);

		// If we fail, drop the client
		if (FAILED(hr))
		{
			_DropClient();
			_SetStatus("Failed to send notification to client.");
		}

		//DEVPRINTF("%d\n", fclib_strlen(szSendBuffer));

		szSendBuffer[0] = 0;
		
		return;
	}

    va_list arglist;
	u32 uLen = fclib_strlen(szSendBuffer);

	if (!uLen)
	{
		fclib_strncpy(szSendBuffer, FSERVER_COMMAND_PREFIX, FSERVER_COMMAND_PREFIX_LEN);
		szSendBuffer[FSERVER_COMMAND_PREFIX_LEN] = '!';

		uLen = FSERVER_COMMAND_PREFIX_LEN + 1;
	}
	else
	{
		szSendBuffer[uLen] = FSERVER_COMMAND_TOKEN;
		++uLen;
	}

	// format arguments
	va_start(arglist, pszFmt);
	_vsnprintf(szSendBuffer + uLen, FSERVER_MAX_SEND_BUFFER - uLen - 1, pszFmt, arglist);
	va_end(arglist);
}

// XBDM_CANNOTACCESS - error code 414 in return buffer - There is a lock
// XBDM_CANNOTCONNECT - error code 400 in return buffer - Someone is using the server

// !! don't stream large amounts of data without bumping up the queue size
//
// This is messy since I need to check and make sure that I don't have another client already connected.
// Since anyone can send commands to the Xbox at anytime, I require my client to send a 'connect' first
// so that they can announce their existence to the server.  I use this 'connect' command to determine
// if we have anyone on the server.  
HRESULT __stdcall CFUpdateServer::_CommandProcessor(LPCSTR pszCommand, LPSTR pszResp,
    DWORD cchResp, PDM_CMDCONT pdmcc)
{
    // skip over prefix
    cchar *pszCmd = pszCommand + FSERVER_COMMAND_PREFIX_LEN + 1;

	// Grab the command that was just sent to us.  It will be executed in Run()
	EnterCriticalSection(&m_hCriticalSection);

	if (m_pServer->m_CommandQueue.IsFull())
	{
		LeaveCriticalSection(&m_hCriticalSection);
		FASSERT(0);

		return XBDM_CANNOTCONNECT;
	}

	if (m_pServer->m_hServerState != _SERVER_STATE_WAIT_CONNECT && 
		!fclib_strncmp("connect|", pszCmd, 8))
	{
		if (m_pServer->_UserLockExists())
		{
			fclib_strncpyz(pszResp, "Server in use! Current user is ", cchResp - 1);

			u32 uLen = fclib_strlen(pszResp);
			s32 uRemain = cchResp - uLen - 1;

			// > 1 since we need a null
			if (uRemain > 1)
				fclib_strncpyz(pszResp + uLen, m_pServer->m_szClientName, uRemain - 1);

			LeaveCriticalSection(&m_hCriticalSection);

			return XBDM_CANNOTCONNECT;
		}
	} 
	else if (m_pServer->m_hServerState == _SERVER_STATE_WAIT_CONNECT && 
		!fclib_strncmp("connect|", pszCmd, 8))
	{
		u32 uLen = fclib_strlen(pszCmd);

		// need at least 9, 8 for connect| and 1 for name
		if (uLen <= 9)
		{
			// !!TODO what should happen?  This means our client was stupid
			FASSERT(0);

			LeaveCriticalSection(&m_hCriticalSection);

			return XBDM_CANNOTCONNECT;
		}

		if (m_pServer->_UserLockExists())
		{
			// assume rest of the command contains the user name
			if (!m_pServer->_UserLockAreUsersSame(pszCmd + 8))
			{
				fclib_strncpyz(pszResp, m_pServer->m_szUserLockName, cchResp - 1);
				LeaveCriticalSection(&m_hCriticalSection);

				return XBDM_CANNOTACCESS;
			}
		}

		// I do this here since if I use a _SendNotification() the client may not get the reply.
		// This is since its notification stuff may not be setup
		if (!m_pServer->m_bBadMasterFile)
		{
			fclib_strncpyz(pszResp, "connect", cchResp - 1); 
		}
		else
		{
			fclib_strncpyz(pszResp, "master_send", cchResp - 1);
		}
	}

	m_pServer->m_CommandQueue.AddToQueue(pszCmd);

	LeaveCriticalSection(&m_hCriticalSection);

    return XBDM_NOERR;
}

void CFUpdateServer::_SetStatus(cchar *pszFmt, ...)
{
    va_list arglist;

    // format arguments
    va_start(arglist, pszFmt);
    _vsnprintf(m_szStatusBuffer[m_sCurrentStatusLine], _STATUS_BUFFER_SIZE, pszFmt, arglist);
    va_end(arglist);

	m_sCurrentStatusLine = (m_sCurrentStatusLine + 1) % _STATUS_BUFFER_COUNT;

	m_bHaveLine = TRUE;
}

// !! This will overwrite the last line in the status buffer.  Make sure you have something there
//    that isn't important.
void CFUpdateServer::_SetStatusNoUpdate(cchar *pszFmt, ...)
{
	s32 sTemp = 0;

	if (m_bHaveLine)
	{
		sTemp = m_sCurrentStatusLine - 1;

		if (sTemp < 0)
			sTemp = _STATUS_BUFFER_COUNT - 1;
	} 

	va_list arglist;

    // format arguments
    va_start(arglist, pszFmt);
    _vsnprintf(m_szStatusBuffer[sTemp], _STATUS_BUFFER_SIZE, pszFmt, arglist);
    va_end(arglist);

	m_bHaveLine = TRUE;
}

#endif