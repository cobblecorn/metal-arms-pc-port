//////////////////////////////////////////////////////////////////////////////////////
// FScriptSystem.h - Script System for Mettle Arms.
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
#ifndef _FSCRIPTSYSTEM_H_
#define _FSCRIPTSYSTEM_H_ 1

#include "fang.h"
#include "amx.h"
#include "fres.h"
#include "FEventListener.h"	// fevent_Bit()
#if !FANG_PRODUCTION_BUILD
#include "ftextmon.h"
#endif

#if !FANG_PRODUCTION_BUILD
#define SCRIPT_ERROR			CFScriptSystem::PrintScriptError
#define SCRIPT_MESSAGE			CFScriptSystem::PrintScriptMessage
#define SCRIPT_CHECK_NUM_PARAMS( pszFunc, nParams ) { s32 nFound = aParams[0] >> 2; if( nFound != (nParams) ) SCRIPT_ERROR( "Check %s Parameters. Found %d, needs %d", (pszFunc), nFound, nParams ); }
#else
#define SCRIPT_ERROR
#define SCRIPT_MESSAGE
#define SCRIPT_CHECK_NUM_PARAMS( pszFunc, nParams )
#endif
class CFStringTable;
class CFScriptInst;
class CFEventListener;
class CFScript;

typedef void EventListenerCallback_t(s32 nWhichEvent, u32 uUserData, u32 uEventData1, u32 uEventData2, u32 uEventData3);




class CFScriptSystem
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	// Called at fang startup time.
	static BOOL ModuleStartup();

	// Called once per invocation of the game (at beginning).  Loads natives.
	static BOOL InitSystem();

	// Called once a level has been selected and loaded.
	// Reads scripts to load and event names to register from the CSV file.
	static BOOL LevelInit(const char *pszCSVFileName);

	// Called once per invocation of the game (at end).  Clears out native list.
	static void UninitSystem();

	// Returns state to before LevelInit.
	static void LevelUninit();

	// 
	static void ModuleShutdown();

	// This resets the system to be as if it had just been started up.
	static BOOL Reset();
	static BOOL RegisterNative(char *pszFuncName, AMX_NATIVE pfcnFunction);

	// Use this method to translate a string into an event number (do this
	//   at init time and save it).
	static s32 GetEventNumFromName(const char *pszEventName);

	// Use this method to get a block of memory that you are using *only* to
	//   pass as a parameter to an event.
	static void *GetMemBlock(u32 uBlockSize);

	// If you adding an object to the world and you want to attach a script to it,
	//   call this method.
	static BOOL AttachObjectToScript(const char *pszScriptFileName, void *pObject);

	// Returns TRUE if all went well.  Returns FALSE if unable to add the listener.
	static BOOL RegisterEventListener(EventListenerCallback_t *pfcnNotify, u32 *phListenerHandle = NULL, u64 *puEventFlags = NULL, u32 uUserData = 0);
	static void SetNotifyForListener(u32 hListenerHandle, s32 nWhichEvent);
	static void StopNotifyForListener(u32 hListenerHandle, s32 nWhichEvent);
	static void SetDisableForListener(u32 hListenerHandle, BOOL bDisable);

	static BOOL DoInitScripts();
	static BOOL DoEndScripts();
	static BOOL Work();
	static void TriggerEvent(s32 nWhichEvent, u32 uEventData1, u32 uEventData2, u32 uEventData3);
	static CFScriptInst *GetScriptInstList( void );
	static CFEventListener *GetEventListenerList( void );
	static FINLINE u32 NumScriptInsts( void ) { return m_uNumScriptInsts; };
	static FINLINE u32 NumEventListeners( void ) { return m_uNumEventListeners; };

	static void PrintScriptError( cchar *pszFormat, ... );
	static void PrintScriptMessage( cchar *pszFormat, ... );
	static void SetMonitorsOn(BOOL bOnOff);
	static BOOL GetMonitorsOn(void);
	static void ClearErrorMonitor( void );
	static void ClearMessageMonitor( void );
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static CFScriptInst *m_pCurScriptInst;
	static AMX_NATIVE_INFO *m_paNativeFunc;
	static u32 m_uNumNatives;
//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL RegisterFangNatives();
	static BOOL RegisterEventNatives();

	static BOOL LoadScriptsFromFile(const char *pszCSVFileName);
	static BOOL LoadEventNamesFromFile(const char *pszCSVFileName);

	static BOOL m_bIsInitialized, m_bLevelInitted;

	static cell AMX_NATIVE_CALL event_Trigger(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL event_SetNotify(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL event_StopNotify(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL event_SetWork(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL event_DevPrintNum(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL event_DevPrintStr(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL event_DevPrintFloat(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL event_TranslateName(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL event_DelayWork(AMX *pAMX, cell *aParams);

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static CFScript *m_paScriptPool;
	static CFScriptInst *m_paScriptInstList;
	static CFEventListener *m_paEventListenerList;
	static AMX_NATIVE_INFO m_aEventNatives[];

	static u32 m_uNumScripts, m_uNumScriptInsts, m_uNumEventListeners;
	static u64 m_uEventsTriggered;

	static FResFrame_t m_hModuleResFrame, m_hLevelResFrame;
	static CFStringTable *m_pEventTable;

#if !FANG_PRODUCTION_BUILD
	static CFTextMonitor m_ErrorMonitor;		// object for displaying script errors
	static CFTextMonitor m_MessageMonitor;		// object for displaying script messages
	static BOOL m_bMonitorsOn;
#endif
	static BOOL m_bSkipWork;					// if TRUE, work function exits early (used to postpone first call to work function at level start)
	/////////////////////////////////////////////////////////////
	// This memory block is used both to queue up events and their data
	static u8 *m_pMemBlock;
	static u8 *m_pNextEvent, *m_pLastData;
	static u32 m_uNumEventsTriggered;
	//
	/////////////////////////////////////////////////////////////

	FCLASS_ALIGN_PREFIX class CEventInst
	{
	public:
		u32 m_uEventNum;
		u32 m_uData1, m_uData2, m_uData3;

		FCLASS_STACKMEM_NOALIGN(CEventInst);
	} FCLASS_ALIGN_SUFFIX;
};

#endif
