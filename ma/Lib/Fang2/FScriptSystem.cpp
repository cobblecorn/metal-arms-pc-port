//////////////////////////////////////////////////////////////////////////////////////
// FScriptSystem.cpp - Script System for Mettle Arms.
//
// Author: Justin Link
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
// 10/24/01 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include <stdio.h>	// for sprintf
#include "FGameData.h"
#include "fclib.h"
#include "FScriptSystem.h"
#include "FScriptTypes.h"
#include "FScriptInst.h"
#include "FEventListener.h"
#include "fres.h"
#include "fstringtable.h"
#include "fgamedata.h"
#include "fresload.h"
#include "FNativeUtil.h"
#include "floop.h"

// =============================================================================================================
// Private definitions:
// =============================================================================================================
static const u32 FScriptSystem_uMaxNatives = 400;

#define _ERROR_MONITOR_LINES				( 7 )
#define _ERROR_MONITOR_LINE_SIZE			( 75 )
#define _ERROR_MONITOR_X_POS				( 0.06f )
#define _ERROR_MONITOR_Y_POS				( 0.3f )
#define _ERROR_MONITOR_LINE_DISPLAY_TIME	( 10.0f )
#define _ERROR_MONITOR_X_SIZE				( 1.0f - (2.0f * _ERROR_MONITOR_X_POS ) )
#define _ERROR_MONITOR_Y_SIZE				( 0.15f )

#define _MESSAGE_MONITOR_LINES				( 5 )
#define _MESSAGE_MONITOR_LINE_SIZE			( 75 )
#define _MESSAGE_MONITOR_X_POS				( 0.06f )
#define _MESSAGE_MONITOR_Y_POS				( 0.6f )
#define _MESSAGE_MONITOR_LINE_DISPLAY_TIME	( 8.0f )
#define _MESSAGE_MONITOR_X_SIZE				( 1.0f - (2.0f * _MESSAGE_MONITOR_X_POS ) )
#define _MESSAGE_MONITOR_Y_SIZE				( 0.107f )

// =============================================================================================================
// Public variables:
// =============================================================================================================

static const u32 _uMemBlockSize = 1024;

//const u32 CFScriptSystem::m_uMaxNatives = 100;
BOOL CFScriptSystem::m_bIsInitialized = FALSE;
BOOL CFScriptSystem::m_bLevelInitted = FALSE;
u32 CFScriptSystem::m_uNumScripts = 0;
u32 CFScriptSystem::m_uNumScriptInsts = 0;
u32 CFScriptSystem::m_uNumEventListeners = 0;
CFScript* CFScriptSystem::m_paScriptPool = NULL;
CFScriptInst* CFScriptSystem::m_paScriptInstList = NULL;
CFEventListener* CFScriptSystem::m_paEventListenerList= NULL;

CFScriptInst *CFScriptSystem::m_pCurScriptInst;
FResFrame_t CFScriptSystem::m_hModuleResFrame, CFScriptSystem::m_hLevelResFrame;
CFStringTable *CFScriptSystem::m_pEventTable;
BOOL CFScriptSystem::m_bSkipWork;

// objects for on-screen display of script errors and messages
#if !FANG_PRODUCTION_BUILD
CFTextMonitor CFScriptSystem::m_ErrorMonitor;
CFTextMonitor CFScriptSystem::m_MessageMonitor;
BOOL CFScriptSystem::m_bMonitorsOn = TRUE;
#endif


AMX_NATIVE_INFO CFScriptSystem::m_aEventNatives[] =
{
	{ "event_Trigger",				CFScriptSystem::event_Trigger },
	{ "event_SetNotify",			CFScriptSystem::event_SetNotify },
	{ "event_StopNotify",			CFScriptSystem::event_StopNotify },
	{ "event_SetWork",				CFScriptSystem::event_SetWork },
	{ "event_DevPrintNum",			CFScriptSystem::event_DevPrintNum },
	{ "event_DevPrintStr",			CFScriptSystem::event_DevPrintStr },
	{ "event_DevPrintFloat",		CFScriptSystem::event_DevPrintFloat },
	{ "event_TranslateName",		CFScriptSystem::event_TranslateName },
	{ "event_DelayWork",			CFScriptSystem::event_DelayWork },
	{ NULL, NULL }
};

u32 CFScriptSystem::m_uNumNatives = 0;
u8 *CFScriptSystem::m_pMemBlock = NULL;
u8 *CFScriptSystem::m_pNextEvent = NULL;
u8 *CFScriptSystem::m_pLastData = NULL;
u32 CFScriptSystem::m_uNumEventsTriggered = 0;
AMX_NATIVE_INFO* CFScriptSystem::m_paNativeFunc = NULL;


#define FSCRIPTSYSTEM_MAXUNIQUESCRIPTS 64
#define FSCRIPTSYSTEM_MAXSCRIPTSINSTS 32
#define FSCRIPTSYSTEM_MAXEVENTLISTENERS 64

// =============================================================================================================
// Public functions:
// =============================================================================================================

BOOL CFScriptSystem::ModuleStartup()
{
	FASSERT(!m_bIsInitialized);

	m_hModuleResFrame = fres_GetFrame();

	m_bIsInitialized = TRUE;
	m_bLevelInitted = FALSE;

	m_pEventTable = CFStringTable::CreateTable("event names");
	m_pMemBlock = (u8 *)(fres_Alloc(_uMemBlockSize));
	m_paNativeFunc = (AMX_NATIVE_INFO*) fres_Alloc(FScriptSystem_uMaxNatives* sizeof (AMX_NATIVE_INFO));
	m_paScriptPool = (CFScript*) fres_Alloc(sizeof(CFScript)*FSCRIPTSYSTEM_MAXUNIQUESCRIPTS);
	m_paScriptInstList = (CFScriptInst*) fres_Alloc(sizeof(CFScriptInst)* FSCRIPTSYSTEM_MAXSCRIPTSINSTS);
	m_paEventListenerList = (CFEventListener*) fres_Alloc(sizeof(CFEventListener)*FSCRIPTSYSTEM_MAXEVENTLISTENERS);

	if (!m_pEventTable ||
		!m_pMemBlock ||
		!m_paNativeFunc ||
		!m_paScriptPool ||
		!m_paScriptInstList ||
		!m_paEventListenerList)
	{
		DEVPRINTF("CFScriptSystem::ModuleStartup() : Could not allocate memory block.\n");
		return FALSE;
	}

	Reset();

	return(TRUE);
}

// =============================================================================================================

void CFScriptSystem::ModuleShutdown()
{
	FASSERT(!m_bLevelInitted);
	m_bIsInitialized = FALSE;

	m_pEventTable = NULL;
	m_pMemBlock = NULL;
	m_paNativeFunc = NULL;
	m_paScriptPool = NULL;
	m_paScriptInstList = NULL;
	m_paEventListenerList = NULL;
	fres_ReleaseFrame(m_hModuleResFrame);
}

// =============================================================================================================

BOOL CFScriptSystem::InitSystem()
{
	m_uNumNatives = 0;
	RegisterEventNatives();
	RegisterFangNatives();

	// objects for on-screen display of script errors and messages
#if !FANG_PRODUCTION_BUILD
	m_ErrorMonitor.Init( _ERROR_MONITOR_LINES, _ERROR_MONITOR_LINE_SIZE );
	m_ErrorMonitor.SetPosition( _ERROR_MONITOR_X_POS, _ERROR_MONITOR_Y_POS );
	m_ErrorMonitor.SetSize( _ERROR_MONITOR_X_SIZE, _ERROR_MONITOR_Y_SIZE );
	m_ErrorMonitor.SetLineDisplayTime( _ERROR_MONITOR_LINE_DISPLAY_TIME );
	m_ErrorMonitor.SetColor( 0.4f, 0.9f, 0.4f );
	m_ErrorMonitor.SetFlashColor( 0.2f, 0.2f, 0.2f );
	m_ErrorMonitor.SetFlashing( TRUE );
	m_ErrorMonitor.SetFlashTime( 4.0f );
	m_MessageMonitor.Init( _MESSAGE_MONITOR_LINES, _MESSAGE_MONITOR_LINE_SIZE );
	m_MessageMonitor.SetPosition( _MESSAGE_MONITOR_X_POS, _MESSAGE_MONITOR_Y_POS );
	m_MessageMonitor.SetSize( _MESSAGE_MONITOR_X_SIZE, _MESSAGE_MONITOR_Y_SIZE );
	m_MessageMonitor.SetLineDisplayTime( _MESSAGE_MONITOR_LINE_DISPLAY_TIME );
	m_MessageMonitor.SetColor( 0.5f, 1.0f, 0.5f );
#endif
	return(TRUE);
}

// =============================================================================================================

void CFScriptSystem::UninitSystem()
{
#if !FANG_PRODUCTION_BUILD
	m_ErrorMonitor.UnInit();
	m_MessageMonitor.UnInit();
#endif
	m_uNumNatives = 0;
}

// =============================================================================================================

BOOL CFScriptSystem::LevelInit(const char *pszCSVFileName)
{
	FASSERT(m_bIsInitialized);
	FASSERT(!m_bLevelInitted);

//	m_hLevelResFrame = fres_GetFrame();

	LoadScriptsFromFile(pszCSVFileName);
	LoadEventNamesFromFile(pszCSVFileName);

	CFST_CFVec3Wrapper::ModuleStartup();
	CFST_MeshWrapper::ModuleStartup();
//	CFST_WorldMeshWrapper::ModuleStartup();
	CFST_Vec3ObjWrapper::ModuleStartup();
	CFST_QuatObjWrapper::ModuleStartup();
	CFST_ScalarObjWrapper::ModuleStartup();

	m_bLevelInitted = TRUE;

	m_bSkipWork = TRUE;
	return(TRUE);
}

// =============================================================================================================

void CFScriptSystem::LevelUninit()
{
	FASSERT(m_bIsInitialized);
	if(!m_bLevelInitted) {
		return;
	}

	m_bLevelInitted = FALSE;

	CFST_ScalarObjWrapper::ModuleShutdown();
	CFST_QuatObjWrapper::ModuleShutdown();
	CFST_Vec3ObjWrapper::ModuleShutdown();
//	CFST_WorldMeshWrapper::ModuleShutdown();
	CFST_MeshWrapper::ModuleShutdown();
	CFST_CFVec3Wrapper::ModuleShutdown();

	u32 uCurScriptIdx;
	for(uCurScriptIdx = 0; uCurScriptIdx < m_uNumScripts; ++uCurScriptIdx)
	{
		m_paScriptPool[uCurScriptIdx].Unload();
	}
	m_uNumScripts = 0;
	m_uNumScriptInsts = 0;
	m_uNumEventListeners = 0;

//	fres_ReleaseFrame(m_hLevelResFrame);
}

// =============================================================================================================

// =============================================================================================================

BOOL CFScriptSystem::Reset()
{
	FASSERT(m_bIsInitialized);

	m_uNumScripts = 0;
	m_uNumScriptInsts = 0;

	m_pNextEvent = m_pMemBlock;
	m_pLastData = m_pMemBlock + _uMemBlockSize;
	m_uNumEventsTriggered = 0;

	return(TRUE);
}

// =============================================================================================================

BOOL CFScriptSystem::RegisterNative(char *pszFuncName, AMX_NATIVE pfcnFunction)
{
	FASSERT(m_bIsInitialized);

	FASSERT(m_uNumNatives < FScriptSystem_uMaxNatives);
	if(m_uNumNatives >= FScriptSystem_uMaxNatives)
	{
		return(FALSE);
	}

	m_paNativeFunc[m_uNumNatives].name = pszFuncName;
	m_paNativeFunc[m_uNumNatives].func = pfcnFunction;
	++m_uNumNatives;

	return(TRUE);
}

// =============================================================================================================

s32 CFScriptSystem::GetEventNumFromName(const char *pszEventName)
{
	if(pszEventName == NULL)
	{
		return(-1);
	}

	s32 nRetValue = m_pEventTable->ComputeStringIndex(pszEventName);

	if(nRetValue == -1)
	{
		DEVPRINTF("CFScriptSystem::GetEventNumFromName : Event '%s' not found in event name table.\n", pszEventName);
	}

	return(nRetValue);
}

// =============================================================================================================

void *CFScriptSystem::GetMemBlock(u32 uBlockSize)
{
	if((m_pLastData - m_pNextEvent) < (s32)(uBlockSize))
	{
		DEVPRINTF("CFScriptSystem::GetMemBlock() : Not enough room to allocate block of size %d.\n", uBlockSize);
		return(NULL);
	}

	m_pLastData -= uBlockSize;
	return(m_pLastData);
}

// =============================================================================================================

BOOL CFScriptSystem::AttachObjectToScript(const char *pszScriptFileName, void *pObject)
{
	FASSERT(m_bIsInitialized);

	u32 uCurScriptIdx;

	// See if we can locate the script file in the pool of already loaded scripts.
	for(uCurScriptIdx = 0; uCurScriptIdx < m_uNumScripts; ++uCurScriptIdx)
	{
		if(fclib_strcmp(pszScriptFileName, m_paScriptPool[uCurScriptIdx].m_szScriptFileName) == 0)
			break;
	}

	if(uCurScriptIdx == m_uNumScripts)
	{
		// We could not find the script already in the pool ...
		// Load the script.
		FASSERT(uCurScriptIdx != FSCRIPTSYSTEM_MAXUNIQUESCRIPTS);
		if(uCurScriptIdx >= FSCRIPTSYSTEM_MAXUNIQUESCRIPTS || m_uNumScriptInsts >= FSCRIPTSYSTEM_MAXSCRIPTSINSTS)
			return(FALSE);
		if(!m_paScriptPool[uCurScriptIdx].LoadFromFile(pszScriptFileName))
			return(FALSE);
		m_uNumScripts++;

		// Initialize the next CFScriptInst with the new script.
		FASSERT(m_uNumScriptInsts != FSCRIPTSYSTEM_MAXSCRIPTSINSTS);
		if(!m_paScriptInstList[m_uNumScriptInsts].Init(&(m_paScriptPool[uCurScriptIdx]), pObject))
			return(FALSE);
	}
	else
	{
		// We did find the script already in the pool ...
		// Find the first CFScriptInst that uses this script.
		u32 uCurScriptInstIdx = 0;
		CFScript *pScript = &(m_paScriptPool[uCurScriptIdx]);

		while(uCurScriptInstIdx < m_uNumScriptInsts && m_paScriptInstList[uCurScriptInstIdx].m_pScript != pScript)
		{
			uCurScriptInstIdx++;
		}
		if(uCurScriptInstIdx == m_uNumScriptInsts)
		{
			// The script loaded but its first instance failed to initialize. amx_Init
			// has already modified the shared program image, so it cannot be retried.
			return(FALSE);
		}

		// Initialize the next CFScriptInst with the previous script instance.
		FASSERT(m_uNumScriptInsts != FSCRIPTSYSTEM_MAXSCRIPTSINSTS);
		if(m_uNumScriptInsts >= FSCRIPTSYSTEM_MAXSCRIPTSINSTS)
			return(FALSE);
		if(!m_paScriptInstList[m_uNumScriptInsts].InitClone(&(m_paScriptInstList[uCurScriptInstIdx]), pObject))
			return(FALSE);
	}

	m_uNumScriptInsts++;

	return(TRUE);
}

// =============================================================================================================

BOOL CFScriptSystem::RegisterEventListener(EventListenerCallback_t *pfcnNotify, u32 *phListenerHandle/* = NULL*/, u64 *puEventFlags/* = NULL*/, u32 uUserData/* = 0*/)
{
	if(m_uNumEventListeners == FSCRIPTSYSTEM_MAXEVENTLISTENERS)
	{
		// No available slots.
		DEVPRINTF("CFScriptSystem::RegisterEventListener() : No available space to add event listener.\n");
		return(FALSE);
	}

	m_paEventListenerList[m_uNumEventListeners].Init(pfcnNotify, uUserData, puEventFlags);
	if(phListenerHandle != NULL)
	{
		// Return them a handle (really a pointer) to the listener in case they want to change parameters on it later.
		*phListenerHandle = (u32)(&(m_paEventListenerList[m_uNumEventListeners]));
	}

	++m_uNumEventListeners;

	return(TRUE);
}

// =============================================================================================================

void CFScriptSystem::SetNotifyForListener(u32 hListenerHandle, s32 nWhichEvent)
{
	CFEventListener *pEL = (CFEventListener *)(hListenerHandle);
	FASSERT(pEL >= &(m_paEventListenerList[0]));
	FASSERT(pEL < &(m_paEventListenerList[m_uNumEventListeners]));

	pEL->SetNotify(nWhichEvent);
}

// =============================================================================================================

void CFScriptSystem::StopNotifyForListener(u32 hListenerHandle, s32 nWhichEvent)
{
	CFEventListener *pEL = (CFEventListener *)(hListenerHandle);
	FASSERT(pEL >= &(m_paEventListenerList[0]));
	FASSERT(pEL < &(m_paEventListenerList[m_uNumEventListeners]));

	pEL->StopNotify(nWhichEvent);
}

// =============================================================================================================

void CFScriptSystem::SetDisableForListener(u32 hListenerHandle, BOOL bDisable)
{
	CFEventListener *pEL = (CFEventListener *)(hListenerHandle);
	FASSERT(pEL >= &(m_paEventListenerList[0]));
	FASSERT(pEL < &(m_paEventListenerList[m_uNumEventListeners]));

	pEL->m_bDisabled = bDisable;
}

// =============================================================================================================

BOOL CFScriptSystem::DoInitScripts()
{
	FASSERT(m_bIsInitialized);
	FASSERT(m_bLevelInitted);

	u32 uCurScriptInstIdx;

	for(uCurScriptInstIdx = 0; uCurScriptInstIdx < m_uNumScriptInsts; ++uCurScriptInstIdx)
	{
		m_pCurScriptInst = &(m_paScriptInstList[uCurScriptInstIdx]);
		if(!m_pCurScriptInst->OnInit())
		{
			DEVPRINTF("Error executing init script for '%s'.\n", m_pCurScriptInst->m_pScript->m_szScriptFileName);
		}
	}
	m_pCurScriptInst = NULL;

	return(TRUE);
}

// =============================================================================================================

BOOL CFScriptSystem::DoEndScripts()
{
	FASSERT(m_bIsInitialized);

	u32 uCurScriptInstIdx;

	for(uCurScriptInstIdx = 0; uCurScriptInstIdx < m_uNumScriptInsts; ++uCurScriptInstIdx)
	{
		m_pCurScriptInst = &(m_paScriptInstList[uCurScriptInstIdx]);
		if(!m_pCurScriptInst->OnEnd())
		{
			DEVPRINTF("Error executing end script #%d.\n", uCurScriptInstIdx);
		}
	}
	m_pCurScriptInst = NULL;

	return(TRUE);
}

// =============================================================================================================

BOOL CFScriptSystem::Work()
{
	FASSERT(m_bIsInitialized);
	
	u32 uCurScriptInstIdx, uCurListenerIdx, uCurEventIdx, uCurEvent;
	u64 uEventsOfInterest;
	CFEventListener *pCurListener;

	if( m_bSkipWork )
	{
		m_bSkipWork = FALSE;
		return TRUE;
	}

	CEventInst *pCurEvent = (CEventInst *)(m_pMemBlock);

	// TODO: Do something about saving off the memory block in case the work
	//   functions below trigger new events or ask for new memory.

	/////////////////////////////////////////////////////////////
	// Dispatch any queued events to the appropriate scripts.
	for(uCurEventIdx = 0; uCurEventIdx < m_uNumEventsTriggered; ++uCurEventIdx)
	{
		/////////////////////////////////////////////////////////////
		// See if any scripts were watching for this event.
		uCurEvent = pCurEvent->m_uEventNum;
		for(uCurScriptInstIdx = 0; uCurScriptInstIdx < m_uNumScriptInsts; ++uCurScriptInstIdx)
		{
			/////////////////////////////////////////////////////////////
			// Execute the OnEvent function.
			m_pCurScriptInst = &(m_paScriptInstList[uCurScriptInstIdx]);
			uEventsOfInterest = m_pCurScriptInst->m_uEventFlags & fevent_Bit(uCurEvent);
			if(uEventsOfInterest != 0)
			{
				if(!m_pCurScriptInst->OnEvent(uCurEvent, pCurEvent->m_uData1, pCurEvent->m_uData2, pCurEvent->m_uData3))
				{
					DEVPRINTF("CFScriptSystem::Work() : Error executing OnEvent function in script %d.\n", uCurScriptInstIdx);
				}
			}
		}

		/////////////////////////////////////////////////////////////
		// See if any listeners wanted to know about this event.
		for(uCurListenerIdx = 0; uCurListenerIdx < m_uNumEventListeners; ++uCurListenerIdx)
		{
			pCurListener = &(m_paEventListenerList[uCurListenerIdx]);
			uEventsOfInterest = pCurListener->m_uEventFlags & fevent_Bit(uCurEvent);
			if(uEventsOfInterest != 0)
			{
				if(!pCurListener->OnEvent(uCurEvent, pCurEvent->m_uData1, pCurEvent->m_uData2, pCurEvent->m_uData3))
				{
					DEVPRINTF("CFScriptSystem::Work() : Error notifying listener of event %d.", uCurEvent);
				}
			}
		}

		++pCurEvent;
	}

	/////////////////////////////////////////////////////////////
	// Reset the memory block for the next frame.  
	// Do this before we call the script work functions, because if
	// The script work functions do something that will trigger an event,
	// we don't want to wipe that event out before it get's processed
	// by wiping out the variables below...
	m_pNextEvent = m_pMemBlock;
	m_pLastData = m_pMemBlock + _uMemBlockSize;
	m_uNumEventsTriggered = 0;

	/////////////////////////////////////////////////////////////
	// Call the work functions of each script.
	for(uCurScriptInstIdx = 0; uCurScriptInstIdx < m_uNumScriptInsts; ++uCurScriptInstIdx)
	{
		m_pCurScriptInst = &(m_paScriptInstList[uCurScriptInstIdx]);

		/////////////////////////////////////////////////////////////
		// If they have set a delay on the work function execution,
		//   update it.
		if(m_pCurScriptInst->m_fDisabledCtdn > 0.0f)
		{
			m_pCurScriptInst->m_fDisabledCtdn -= FLoop_fPreviousLoopSecs;
		}

		/////////////////////////////////////////////////////////////
		// If the Work function has not been disabled and is not currently paused, execute it.
		if(((m_pCurScriptInst->m_uFlags & CFSCRIPTINST_FLAG_DOWORK) != 0) && (m_pCurScriptInst->m_fDisabledCtdn <= 0.0f))
		{
			if(!m_pCurScriptInst->Work())
			{
				DEVPRINTF("CFScriptSystem::Work() : Error executing Work function in script %d.\n", uCurScriptInstIdx);
			}
		}
	}

	m_pCurScriptInst = NULL;

#if !FANG_PRODUCTION_BUILD
	m_ErrorMonitor.DisplayWork();
	m_MessageMonitor.DisplayWork();
#endif
	return(TRUE);
}

// =============================================================================================================

void CFScriptSystem::TriggerEvent(s32 nWhichEvent, u32 uEventData1, u32 uEventData2, u32 uEventData3)
{
	FASSERT(m_bIsInitialized);
	FASSERT(m_bLevelInitted);

	FASSERT(nWhichEvent <= 63);
	FASSERT(nWhichEvent >= -1);
	if(nWhichEvent == -1)
	{
		return;
	}

	/////////////////////////////////////////////////////////////
	// Check to see if we have enough room to record the event.
	if((m_pLastData - m_pNextEvent) < sizeof(CEventInst))
	{
		DEVPRINTF("CFScriptSystem::TriggerEvent : Memory block size of %d has been exhausted.  Cannot record event %d.\n", _uMemBlockSize, nWhichEvent);
		return;
	}
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Add the event to our event queue.
	CEventInst *pEI = (CEventInst *)(m_pNextEvent);
	pEI->m_uEventNum = nWhichEvent;
	pEI->m_uData1 = uEventData1;
	pEI->m_uData2 = uEventData2;
	pEI->m_uData3 = uEventData3;

	m_pNextEvent += sizeof(CEventInst);
	++m_uNumEventsTriggered;
	//
	/////////////////////////////////////////////////////////////
}

// =============================================================================================================
// DO NOT call this function directly, use the macro SCRIPT_ERROR() instead
// Otherwise the text will get printed in a production build.
void CFScriptSystem::PrintScriptError( cchar *pszFormat, ... )
{
#if FANG_PLATFORM_DX && !FANG_PRODUCTION_BUILD
	char szFormat[512];
	char szError[512];
	BOOL bPrintToMonitor = TRUE;

	if( CFTextMonitor::IsPaused() )
	{
		// don't show additional errors when paused
		bPrintToMonitor = FALSE;
	}

	if( !m_bMonitorsOn )
	{
		// don't print errors (and thus pause the game) when monitors are off
		bPrintToMonitor = FALSE;
	}

	FANG_VA_LIST args;
	FANG_VA_START( args, pszFormat );

	// can't pass on args to AddLine or DEVPRINTF for some reason, so expand string here.
	_vsnprintf( szError, 511, pszFormat, args );
	szError[511] = 0;

	if( bPrintToMonitor )
	{
		CFScriptSystem::m_ErrorMonitor.AddLine( "A Script Error has occurred:" );
	}

	if (CFScriptSystem::m_pCurScriptInst && CFScriptSystem::m_pCurScriptInst->m_pScript && CFScriptSystem::m_pCurScriptInst->m_pScript->m_szScriptFileName )
	{
		sprintf( szFormat, "%s %d ", CFScriptSystem::m_pCurScriptInst->m_pScript->m_szScriptFileName, m_pCurScriptInst->m_oAMX.curline );
	}
	else
	{
		sprintf( szFormat, "NONE");
	}
	fclib_strcat( szFormat, szError );

	if( bPrintToMonitor )
	{
		CFScriptSystem::m_ErrorMonitor.AddLine( szFormat, args );
	}

	DEVPRINTF( "SCRIPT ERROR:" );
	DEVPRINTF( szFormat );
	DEVPRINTF( "\n" );

	if( bPrintToMonitor )
	{
		CFScriptSystem::m_ErrorMonitor.AddLine( "Press the Jump button to continue." );
	}

	FANG_VA_END( args );

	if( bPrintToMonitor )
	{
		CFTextMonitor::Pause( TRUE );
	}
#endif
}

// =============================================================================================================
// DO NOT call this function directly, use the macro SCRIPT_MESSAGE() instead
// Otherwise the text will get printed in a production build.
void CFScriptSystem::PrintScriptMessage( cchar *pszFormat, ... )
{
#if FANG_PLATFORM_DX && !FANG_PRODUCTION_BUILD
	char szFormat[512];
	char szError[512];

	FANG_VA_LIST args;
	FANG_VA_START( args, pszFormat );

	// can't pass on args to AddLine or DEVPRINTF for some reason, so expand string here.
	_vsnprintf( szError, 511, pszFormat, args );
	szError[511] = 0;

	if (CFScriptSystem::m_pCurScriptInst && CFScriptSystem::m_pCurScriptInst->m_pScript && CFScriptSystem::m_pCurScriptInst->m_pScript->m_szScriptFileName )
	{
		sprintf( szFormat, "%s %d ", CFScriptSystem::m_pCurScriptInst->m_pScript->m_szScriptFileName, m_pCurScriptInst->m_oAMX.curline );
	}
	else
	{
		sprintf( szFormat, "NONE");
	}
	fclib_strcat( szFormat, szError );

	if( m_bMonitorsOn )
	{
		CFScriptSystem::m_MessageMonitor.AddLine( szFormat, args );
	}

	DEVPRINTF( "SCRIPT MESSAGE:" );
	DEVPRINTF( szFormat );
	DEVPRINTF( "\n" );
	FANG_VA_END( args );
#endif
}

// =============================================================================================================

cell AMX_NATIVE_CALL CFScriptSystem::event_Trigger(AMX *pAMX, cell *aParams)
{
	TriggerEvent(aParams[1], aParams[2], aParams[3], aParams[4]);
	return(0);
}

// =============================================================================================================

cell CFScriptSystem::event_SetNotify(AMX *pAMX, cell *aParams)
{
	m_pCurScriptInst->m_uEventFlags |= fevent_Bit(aParams[1]);
	return(0);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CFScriptSystem::event_StopNotify(AMX *pAMX, cell *aParams)
{
	m_pCurScriptInst->m_uEventFlags &= ~fevent_Bit(aParams[1]);
	return(0);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CFScriptSystem::event_SetWork(AMX *pAMX, cell *aParams)
{
	if(aParams[1] == 0)
		m_pCurScriptInst->m_uFlags &= ~(CFSCRIPTINST_FLAG_DOWORK);
	else
		m_pCurScriptInst->m_uFlags |= CFSCRIPTINST_FLAG_DOWORK;
	return(0);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CFScriptSystem::event_DevPrintNum(AMX *pAMX, cell *aParams)
{
	SCRIPT_MESSAGE( "Script Number: %d", aParams[1] );
	return(0);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CFScriptSystem::event_DevPrintStr(AMX *pAMX, cell *aParams)
{
	char szBuf[1024];

	FillStringFromCell( szBuf, 1023, pAMX, aParams[1] );
	SCRIPT_MESSAGE( "Script String: %s", szBuf );
	return(0);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CFScriptSystem::event_DevPrintFloat(AMX *pAMX, cell *aParams)
{
//	char szBuf[1024];

	SCRIPT_MESSAGE( "Script Float: %f", ConvertCellToF32(aParams[1]) );
	return(0);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CFScriptSystem::event_TranslateName(AMX *pAMX, cell *aParams)
{
	char szEventName[200];
	FillStringFromCell(szEventName, 199, pAMX, aParams[1]);
	return(CFScriptSystem::GetEventNumFromName(szEventName));
}

// =============================================================================================================

cell AMX_NATIVE_CALL CFScriptSystem::event_DelayWork(AMX *pAMX, cell *aParams)
{
	m_pCurScriptInst->m_fDisabledCtdn = ConvertCellToF32(aParams[1]);

	return((cell)(0));
}

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

BOOL CFScriptSystem::RegisterFangNatives()
{
	FASSERT(m_bIsInitialized);

	AMX_NATIVE_INFO *pCurNative = &(FScriptTypes_aFangEventNatives[0]);
	while(pCurNative->func != NULL)
	{
		if(!RegisterNative(pCurNative->name, pCurNative->func))
			return(FALSE);
		++pCurNative;
	}

	return(TRUE);
}

// =============================================================================================================

BOOL CFScriptSystem::RegisterEventNatives()
{
	FASSERT(m_bIsInitialized);

	FASSERT(m_uNumNatives <= FScriptSystem_uMaxNatives);
	if(m_uNumNatives == FScriptSystem_uMaxNatives)
		return(FALSE);

	u32 uEventNativeIdx = 0;
	while(m_aEventNatives[uEventNativeIdx].func != NULL)
	{
		if(!RegisterNative(m_aEventNatives[uEventNativeIdx].name, m_aEventNatives[uEventNativeIdx].func))
			return(FALSE);
		++uEventNativeIdx;
	}

	return(TRUE);
}

// =============================================================================================================

#define _SCRIPTS_TABLE_NAME		"scripts"

BOOL CFScriptSystem::LoadScriptsFromFile( const char *pszCSVFileName ) {
	u32 i, nNumScripts;
	cchar *pszScriptName;
	FGameData_VarType_e nDataType;
	FGameDataTableHandle_t hTableHandle;
	
	FASSERT(m_bIsInitialized);

	if( !pszCSVFileName ) {
		return TRUE;
	}

	FMemFrame_t Frame = fmem_GetFrame();

	// Read the .CSV file here.
	FGameDataFileHandle_t hFile = (FGameDataFileHandle_t)fresload_Load( FGAMEDATA_RESTYPE, pszCSVFileName );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		goto _ExitWithError;
	}

	// find the table we are interested in
	hTableHandle = fgamedata_GetFirstTableHandle( hFile, _SCRIPTS_TABLE_NAME );
	if( hTableHandle == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CFScriptSystem::LoadScriptsFromFile : No script names found for this level.\n" );
		goto _ExitWithError;
	}

	// get the number of fields in the table
	nNumScripts = fgamedata_GetNumFields( hTableHandle );
	if( !nNumScripts ) {
		DEVPRINTF( "CFScriptSystem::LoadScriptsFromFile : No script names found for this level.\n" );
		goto _ExitWithError;
	}

	for( i=0; i < nNumScripts; i++ ) {
		pszScriptName = (cchar *)fgamedata_GetPtrToFieldData( hTableHandle, i, nDataType );
		if( pszScriptName && nDataType == FGAMEDATA_VAR_TYPE_STRING ) {
			DEVPRINTF( "Found script file in CSV: %s.\n", pszScriptName );
			if( !CFScriptSystem::AttachObjectToScript( pszScriptName, NULL ) ) {
				DEVPRINTF( "ERROR: [fScriptSystem] Error attaching object to script '%s'.\n", pszScriptName );
//				goto _ExitWithError;
			}
		}
	}

	fmem_ReleaseFrame( Frame );

	return TRUE;

	// Error:
_ExitWithError:
	fmem_ReleaseFrame( Frame );
	return FALSE;
}

// =============================================================================================================

#define _EVENTS_TABLE_NAME	"events"

BOOL CFScriptSystem::LoadEventNamesFromFile( const char *pszCSVFileName ) {
	u32 i, nNumEvents;
	cchar *pszEventName;
	FGameData_VarType_e nDataType;
	FGameDataTableHandle_t hTableHandle;

	FASSERT(m_bIsInitialized);
	
	if( !pszCSVFileName ) {
		return TRUE;
	}

	FMemFrame_t Frame = fmem_GetFrame();

	// Read the .CSV file here.
	FGameDataFileHandle_t hFile = (FGameDataFileHandle_t)fresload_Load( FGAMEDATA_RESTYPE, pszCSVFileName );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		goto _ExitWithError;
	}

	// find the table we are interested in
	hTableHandle = fgamedata_GetFirstTableHandle( hFile, _EVENTS_TABLE_NAME );
	if( hTableHandle == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF("CFScriptSystem::LoadEventNamesFromFile : No event names found for this level.\n");
		goto _ExitWithError;
	}
	
	// get the number of fields in the table
	nNumEvents = fgamedata_GetNumFields( hTableHandle );
	if( !nNumEvents ) {
		DEVPRINTF("CFScriptSystem::LoadEventNamesFromFile : No event names found for this level.\n");
		goto _ExitWithError;
	}

	for( i=0; i < nNumEvents; i++ ) {
		pszEventName = (cchar *)fgamedata_GetPtrToFieldData( hTableHandle, i, nDataType );

		if( pszEventName && nDataType == FGAMEDATA_VAR_TYPE_STRING ) {
			DEVPRINTF("Found event name in CSV: %s.\n", pszEventName );

			if( m_pEventTable->GetStringCount() == 64 ) {
				DEVPRINTF("CFScriptSystem::LoadEventNamesFromFile : Event table is full.  Cannot add any more entries.\n");
			} else if( m_pEventTable->AddString( pszEventName, TRUE ) == NULL ) {
				DEVPRINTF("CFScriptSystem::LoadEventNamesFromFile : Event '%s' already in table.\n", pszEventName);
			}
		}
	}

	fmem_ReleaseFrame( Frame );

	return TRUE;

	// Error:
_ExitWithError:
	fmem_ReleaseFrame( Frame );
	return FALSE;
}

// =============================================================================================================
BOOL CFScriptSystem::GetMonitorsOn(void)
{
#if !FANG_PRODUCTION_BUILD
	return m_bMonitorsOn;
#else
	return FALSE;
#endif
}

// =============================================================================================================
void CFScriptSystem::SetMonitorsOn(BOOL bOnOff)
{
#if !FANG_PRODUCTION_BUILD
	m_bMonitorsOn = !!bOnOff;

	if( m_bLevelInitted ) {
		m_ErrorMonitor.SetDisplay( m_bMonitorsOn );
		m_MessageMonitor.SetDisplay( m_bMonitorsOn );
	}
#endif
}

// =============================================================================================================
void CFScriptSystem::ClearErrorMonitor( void )
{
#if !FANG_PRODUCTION_BUILD
	m_ErrorMonitor.ClearDisplay();
#endif
}

// =============================================================================================================
void CFScriptSystem::ClearMessageMonitor( void )
{
#if !FANG_PRODUCTION_BUILD
	m_MessageMonitor.ClearDisplay();
#endif
}

CFScriptInst *CFScriptSystem::GetScriptInstList( void )
{
	return &(m_paScriptInstList[0]);
};


CFEventListener *CFScriptSystem::GetEventListenerList( void )
{
	return &(m_paEventListenerList[0]);
};
