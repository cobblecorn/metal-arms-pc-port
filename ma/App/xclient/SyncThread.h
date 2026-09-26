#ifndef _SYNCTHREAD_H_
#define _SYNCTHREAD_H_ 1

#include "fang.h"
#include "fdata.h"
#include "xboxdbg.h"
#include "dx/fserver.h"
#include "fmasterfile.h"

class CXclientDlg;

class CCommandQueue
{
public:
	CCommandQueue();

	BOOL IsEmpty(void) {return m_bEmpty;}
	BOOL IsFull(void)  {return !m_bEmpty && (m_uHead == m_uTail);}
	void Clear(void) {m_uHead = m_uTail = 0; m_bEmpty = true;}
	BOOL AddToQueue(cchar *pszCommand);
	// !!TODO add size param?
	void RemoveFromQueue(char *pszDest, const s32 &sMaxChar);

private:
	enum
	{
		_COMMAND_QUEUE_SIZE = 10
	};

	// If queue gets larger, use ints
	u8 m_uHead;
	u8 m_uTail;
	BOOL m_bEmpty;
	char m_szQueue[_COMMAND_QUEUE_SIZE][FSERVER_MAX_SEND_BUFFER];
};

class CSyncThread
{
public:
	// Must call before anything else to use thread
	static void Reset(void);

	// Call these before you start the thread
	static void SetParent(CXclientDlg *pParent)         {m_pParent = pParent;}
	static void SetXboxName(const CString &sStr)		{m_sXboxName = sStr;}
	static void SetUserName(const CString &sStr)		{m_sUserName = sStr;}
	static void SetLocalMasterFile(const CString &sStr) {m_sLocalMasterFile = sStr;}
	static void SetLaunchTitle(const CString &sStr)     {m_sLaunchTitle = sStr;}
	static void SetLaunchOnly(BOOL bLO)                 {m_bLaunchOnly = bLO;}
	static void SetCloseOnFinish(BOOL bClose)           {m_bClose = bClose;}

	static void StopThread(void);
	static BOOL ThreadRunning(void)                     {return m_bRunThread;}

	static UINT _ThreadFunc(LPVOID pParam);
	
private:
	enum 
	{
		_SYNC_STATE_NONE = 1,			// Doing nothing
		_SYNC_STATE_GET_MASTER_INFO,	// Getting master information from the Xbox
		_SYNC_STATE_SEND_MASTER_INFO,	// Sending out master info updates to the Xbox
		_SYNC_STATE_SEND_FILES,         // Sending out files
	};

	// !!!!! Must set before the thread starts
	static CXclientDlg *m_pParent;			// Parent window that called us
	static CString m_sXboxName;				// Xbox to connect to
	static CString m_sUserName;				// User name to connect to the Xbox with
	static CString m_sLocalMasterFile;		// Path to the local master file

	static CString m_sLaunchTitle;			// Title we should launch
	// !!TODO not used yet
	static BOOL m_bShutdown;				// Shutdown the server?
	static BOOL m_bLaunchOnly;				// Launch a game only?
	static BOOL m_bClose;
	// !!!!! End !!!!!

	static BOOL m_bRunThread;				// Should we run the thread
	static u8 m_hSyncState;					// State we are in

	static HANDLE m_hMutex;			
	static CCommandQueue m_CommandQueue;	// Command queue

	static PDM_CONNECTION m_XboxConnection; // Connection to the Xbox
	static PDMN_SESSION   m_XboxSession;    // Session to the Xbox for notification messages

	static u32 m_uExpectedNumFileEntries;	// How many file entries are we expecting from Xbox
	static u32 m_uNumFileEntries;			// How many file entries have we got so far from Xbox
	static FDataPrjFile_Entry_t m_FileEntries[FDATA_PRJFILE_MIN_ENTRY_COUNT]; // File entries from Xbox
	
	static CFMasterFile masterFile;
	static u32 m_uNumFileEntriesSent;		// File entries that we have sent
	static u32 m_uNumFileEntriesNeedUpdate; // File entries that need an update
	static const FDataPrjFile_Entry_t *m_FileEntriesNeedUpdate[FDATA_PRJFILE_MIN_ENTRY_COUNT];

	static DWORD __stdcall _NotifyFunc(LPCSTR pszNotification);

	// All these methods are surrounded by a enter / leave critical section
	static BOOL _Connect(void);
	static BOOL _Disconnect(BOOL bSendDisconnect = TRUE);
	static BOOL _SendVersion(void);
	static BOOL _RequestMasterFileInfo(void);
	static BOOL _ProcessCommand(char *pszCmd);
	static BOOL _SetExpectedNumEntries(const u32 &uArgc, char *pArgv[]);
	static BOOL _ProcessEntry(const u32 &uArgc, char *pArgv[]);
	static BOOL _DetermineNeedUpdateFiles(const u32 &uArgc, char *pArgv[]);
	
	static BOOL _GetFilesNeedUpdate(void);
	static BOOL _SendNeedUpdateFiles(void);
	static BOOL _SendNextFile(void);
	static void _SendNextMasterEntry(void);

	static BOOL _Done(const u32 &uArgc, char *pArgv[]);

	static BOOL _MasterFileSend(void);
	static BOOL _MasterFileShouldSend(void);

	// end methods w/ critical section

	static void _LaunchTitle(void);
	static void _ForceRebootServer(void);

	static FDataPrjFile_Entry_t *_GetXboxEntryByName(const char *pszName);

	static BOOL _SendText(const char *pszFmt, ...);
	static void _SetStatusText(const CString &sStatus);
};

#endif