#include <afx.h>
#include <afxcmn.h>
#include "fang.h"
#include "SyncThread.h"
#include "xclient.h"
#include "xclientDlg.h"
#include "fclib.h"

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

// !!!!!!! uses a | to parse between elements.  Taken from Xbox SDK sample program.
static u32 _CmdToArgv(char *szCmd, char *szArg[], const u32 &maxargs, const char &cToken)
{
    u32 argc = 0;
    u32 argcT = 0;
    char *szNil = szCmd + lstrlenA(szCmd);

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
            char chEnd = (*szCmd == '"' || *szCmd == '\'') ? *szCmd++ : cToken;
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

static int _QSortCompareFcn( const void *arg1, const void *arg2 )
{
	FDataPrjFile_Entry_t *p1, *p2;

	p1 = (FDataPrjFile_Entry_t *) arg1;
	p2 = (FDataPrjFile_Entry_t *) arg2;

   // Compare all of both strings:
   return fclib_stricmp(p1->szFilename, p2->szFilename);
}

CXclientDlg *CSyncThread::m_pParent = 0;

CString CSyncThread::m_sXboxName;
CString CSyncThread::m_sUserName;
CString CSyncThread::m_sLocalMasterFile;

CString CSyncThread::m_sLaunchTitle;
BOOL CSyncThread::m_bLaunchOnly;
BOOL CSyncThread::m_bClose;

BOOL CSyncThread::m_bRunThread;
u8 CSyncThread::m_hSyncState;

HANDLE CSyncThread::m_hMutex = 0;
CCommandQueue CSyncThread::m_CommandQueue;

PDM_CONNECTION CSyncThread::m_XboxConnection;
PDMN_SESSION   CSyncThread::m_XboxSession;

u32 CSyncThread::m_uExpectedNumFileEntries; 
u32 CSyncThread::m_uNumFileEntries; 
FDataPrjFile_Entry_t CSyncThread::m_FileEntries[FDATA_PRJFILE_MIN_ENTRY_COUNT]; // File entries from Xbox

CFMasterFile CSyncThread::masterFile;
u32 CSyncThread::m_uNumFileEntriesSent;
u32 CSyncThread::m_uNumFileEntriesNeedUpdate;
const FDataPrjFile_Entry_t *CSyncThread::m_FileEntriesNeedUpdate[FDATA_PRJFILE_MIN_ENTRY_COUNT];

void CSyncThread::Reset(void)
{
	if (m_hMutex)
	{
		CloseHandle(m_hMutex);
	}

	m_pParent = 0;

	m_hMutex = CreateMutex(NULL, FALSE, NULL);

	m_sXboxName = "";
	m_sUserName = "";
	m_sLocalMasterFile = "";
	m_sLaunchTitle = "";
	m_bLaunchOnly = FALSE;
	m_bRunThread = FALSE;
	m_bClose = FALSE;
	m_hSyncState = _SYNC_STATE_NONE;

	m_XboxConnection = 0;
	m_XboxSession = 0;

	m_uExpectedNumFileEntries = 0;
	m_uNumFileEntries = 0;
		
	masterFile.Reset();

	m_uNumFileEntriesSent = 0;
	m_uNumFileEntriesNeedUpdate = 0;

	m_CommandQueue.Clear();

	//DmUseSharedConnection(TRUE);
}

void CSyncThread::StopThread(void)
{
	if (WaitForSingleObject(m_hMutex, INFINITE) == WAIT_FAILED)
	{
		return;
	}

	m_bRunThread = FALSE;

	ReleaseMutex(m_hMutex);
}

UINT CSyncThread::_ThreadFunc(LPVOID pParam)
{
	FASSERT(m_pParent);

	m_pParent->StatusTextClear();

	if (!m_bLaunchOnly)
	{
		if (!masterFile.Load(m_sLocalMasterFile))
		{
			_SetStatusText("FATAL ERROR: Failed to load master file " + m_sLocalMasterFile + "!\n");
			Reset();
			return 0;
		}

		if (!_Connect())
		{
			Reset();
			return 0;
		}

		m_bRunThread = TRUE;

		char m_szBuffer[FSERVER_MAX_SEND_BUFFER];

		while (m_bRunThread)
		{
			if (WaitForSingleObject(m_hMutex, INFINITE) == WAIT_FAILED)
			{
				return 0;
			}

			while (m_bRunThread && !m_CommandQueue.IsEmpty())
			{
				m_CommandQueue.RemoveFromQueue(m_szBuffer, FSERVER_MAX_SEND_BUFFER);
				
				if (!_ProcessCommand(m_szBuffer))
				{
					m_bRunThread = FALSE;
				}
			}

			if (m_hSyncState == _SYNC_STATE_SEND_FILES)
			{
				_SendNextFile();
			}

			ReleaseMutex(m_hMutex);
		}

		_Disconnect();

		CloseHandle(m_hMutex);
		m_hMutex = 0;
	}

	// Only force send files once
	std::vector<CString> &FileList = m_pParent->GetFileList();
	FileList.clear();

	masterFile.Reset();

	if (!m_sLaunchTitle.IsEmpty())
	{
		_LaunchTitle();
	}

	m_bRunThread = FALSE;

	if (m_bClose)
		m_pParent->SendMessage(WM_CLOSE);

	return 0;
}

DWORD __stdcall CSyncThread::_NotifyFunc(LPCSTR pszNotification)
{
	WaitForSingleObject(CSyncThread::m_hMutex, INFINITE);
	
	if (!CSyncThread::m_CommandQueue.AddToQueue(pszNotification))
	{
		// !!TODO 
		FASSERT(0);
	}
	
	ReleaseMutex(CSyncThread::m_hMutex);

	return 0;
}

BOOL CSyncThread::_Connect(void)
{
	HRESULT hr;

	_SetStatusText("Trying to connect to Xbox " + m_sXboxName + "...\n");

	hr = DmSetXboxNameNoRegister(m_sXboxName);

	if (FAILED(hr))
	{
		_SetStatusText("Failed to set Xbox to " + m_sXboxName + "!\n");
		return FALSE;
	}

	 // Open our connection
    hr = DmOpenConnection(&m_XboxConnection);

    if (FAILED(hr))
    {
        _SetStatusText("Failed to connect to Xbox!\n");
		return FALSE;
    }

	DWORD cchResp = FSERVER_MAX_SEND_BUFFER;
    char szResp[FSERVER_MAX_SEND_BUFFER];
	CString sCommand = CString(FSERVER_COMMAND_PREFIX) + "!connect|" + m_sUserName;

    hr = DmSendCommand(m_XboxConnection, sCommand, szResp, &cchResp);

    if (FAILED(hr))
    {
		int sRes = IDNO;

		// XBDM_CANNOTACCESS - error code 414 in return buffer - There is a lock
		// XBDM_CANNOTCONNECT - error code 400 in return buffer - Someone is using the server

		if (szResp[0] == '4' && szResp[1] == '1' && szResp[2] == '4')
		{
			sRes  = m_pParent->MessageBox("Server is currently in use.  Would you like to kick the current user?", 
				"Server in use!", MB_ICONEXCLAMATION | MB_YESNO);

			if (sRes == IDYES)
			{
				_SetStatusText("Booting current user from the server.\n");
				_SetStatusText("Trying to reconnect...\n");	

				// boot em
				_SendText("boot");

				DmCloseConnection(m_XboxConnection);
				m_XboxConnection = 0;

				Sleep(2 * 1000);

				return _Connect();
			}
		}
		else if (szResp[0] == '4' && szResp[1] == '0' && szResp[2] == '7')
		{
			_ForceRebootServer();

			_SetStatusText("Trying to reconnect...\n");

			DmCloseConnection(m_XboxConnection);
			m_XboxConnection = 0;

			Sleep(2 * 1000);

			return _Connect();
		}
		else
		{
			_SetStatusText("Connection to " + m_sXboxName + " failed with error: " + szResp + "\n");
		}

		_Disconnect(FALSE);

		return FALSE;
	}

	hr = DmOpenNotificationSession(0, &m_XboxSession);

	if (FAILED(hr))
	{
		_SetStatusText("Unable to open notification session\n");
		_Disconnect();
		return FALSE;
	}

	hr = DmRegisterNotificationProcessor(m_XboxSession, FSERVER_COMMAND_PREFIX, _NotifyFunc);

	if (FAILED(hr))
	{
		_SetStatusText("Unable to register notification processor\n");
		_Disconnect();
		return FALSE;
	}

	_SetStatusText("Connected to Xbox " + m_sXboxName + "!\n");

	if (!fclib_strcmp(szResp, "200- connect"))
	{
		//return _RequestMasterFileInfo();
		//return _SendVersion();
		return TRUE;
	}
	else if (!fclib_strcmp(szResp, "200- master_send"))
	{
		_SetStatusText("Xbox believes it has a bad masterfile and has requested the entire masterfile.\n");
		if (!_MasterFileSend())
		{
			_Disconnect();
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CSyncThread::_Disconnect(BOOL bSendDisconnect)
{
	if (m_XboxConnection && bSendDisconnect)
	{
		_SendText("disconnect");
	}

	if (m_XboxSession)
	{
		DmCloseNotificationSession(m_XboxSession);
	}

	if (m_XboxConnection)
	{
		DmCloseConnection(m_XboxConnection);
	}

	//DmUseSharedConnection(FALSE);

	m_XboxConnection = 0;
	m_XboxSession = 0;

	m_bRunThread = FALSE;

	return TRUE;
}

BOOL CSyncThread::_SendVersion(void)
{
	return _SendText("version|%d|%d|%d|%d|%d|%d|%d|%d|%d",
		masterFile.GetTgaVersionFromHeader(),
		masterFile.GetApeVersionFromHeader(),
		masterFile.GetMtxVersionFromHeader(),
		masterFile.GetCsvVersionFromHeader(),
		masterFile.GetFntVersionFromHeader(),
		masterFile.GetSmaVersionFromHeader(),
		masterFile.GetGtVersionFromHeader(),
		masterFile.GetWvbVersionFromHeader(),
		masterFile.GetFprVersionFromHeader());
}

BOOL CSyncThread::_RequestMasterFileInfo(void)
{
	if (!_SendText("get_master_info"))
	{
		_SetStatusText("Request for master file information failed!\n");
		return FALSE;
	}

	return TRUE;
}

BOOL CSyncThread::_ProcessCommand(char *pszCmd)
{
	u32 uNumCommands;
	char *pCommands[FSERVER_MAX_COMMANDS];
	char *pszBuffer = pszCmd + FSERVER_COMMAND_PREFIX_LEN + 1;

	uNumCommands = _CmdToArgv(pszBuffer, pCommands, FSERVER_MAX_COMMANDS, FSERVER_COMMAND_TOKEN);

	if (uNumCommands <= 0)
		return FALSE;

	u32 uArgc;
	char *pArgv[FSERVER_MAX_COMMAND_ARGUMENTS];

	for (u32 uCnt = 0; uCnt < uNumCommands; ++uCnt)
	{
		uArgc = _CmdToArgv(pCommands[uCnt], pArgv, FSERVER_MAX_COMMAND_ARGUMENTS, FSERVER_ITEM_TOKEN);
		
		if (uArgc <= 0)
			return FALSE;

		// If we are told to disconnect, abort everything
		if (!fclib_strncmp("disconnect", pArgv[0], 10))
		{
			_SetStatusText("Disconnected from server!");
			_Disconnect(FALSE);

			break;
		}

		switch (m_hSyncState)
		{
			case _SYNC_STATE_NONE:
				if (!fclib_strncmp("num_entries", pArgv[0], 11))
				{
					if (!_SetExpectedNumEntries(uArgc, pArgv))
					{
						return FALSE;
					}
				}
				else if (!fclib_strncmp("done", pArgv[0], 4))
				{
					_SetStatusText("Xbox reports it is done.\n");
					if (!_Done(uArgc, pArgv))
					{
						return FALSE;
					}
				}
				else if (!fclib_strncmp("work", pArgv[0], 4))
				{
					m_pParent->ProgressStep();
				}
				else if (!fclib_strncmp("master_send", pArgv[0], 11))
				{
					_SetStatusText("Xbox has requested the entire masterfile.\n");
					if (!_MasterFileSend())
						return FALSE;
				}
				else if (!fclib_strncmp("version", pArgv[0], 7))
				{
					_SetStatusText("Xbox has requested version information.  Sending info...\n");
					return _SendVersion();
				}
				else if (!fclib_strncmp("ok_version", pArgv[0], 10))
				{
					_SetStatusText("Xbox has verfied a version match.\n");
					return _RequestMasterFileInfo();
				}
			break;
			case _SYNC_STATE_GET_MASTER_INFO:
				// NKM - Fixed the 4, should have been 5 - Stupid
				if (!fclib_strncmp("entry", pArgv[0], 5))
				{
					if (!_ProcessEntry(uArgc, pArgv))
					{
						return FALSE;
					}
				}
				else if (!fclib_strncmp("get_master_info_done", pArgv[0], 20))
				{
					if (!_DetermineNeedUpdateFiles(uArgc, pArgv))
						return FALSE;
				}
			break;
			case _SYNC_STATE_SEND_MASTER_INFO:
				if (!fclib_strncmp("ok", pArgv[0], 2))
				{
					_SendNextMasterEntry();	
				}
			break;
		}	
	}
	// return true even if we get a garbage command?

	return TRUE;
}

BOOL CSyncThread::_SetExpectedNumEntries(const u32 &uArgc, char *pArgv[])
{
	if (uArgc < 2)
	{
		_SetStatusText("FATAL ERROR: Bad number of arguments to _SetExpectedNumEntries()!\n");
		return FALSE;
	}

	m_uExpectedNumFileEntries = atoi(pArgv[1]);
	m_uNumFileEntries = 0;
	m_hSyncState = _SYNC_STATE_GET_MASTER_INFO;

	if (m_uExpectedNumFileEntries <= 0)
	{
		_SetStatusText("FATAL ERROR: Expected file entries is <= 0!\n");
		return FALSE;
	}

	CString sNum;

	sNum.Format("%d", m_uExpectedNumFileEntries);

	_SetStatusText("Getting master file information from Xbox.  ");
	_SetStatusText("Expecting " + sNum + " entries...\n");

	m_pParent->SetProgressStep(1);
	m_pParent->SetProgressRange(0, m_uExpectedNumFileEntries);
	m_pParent->SetProgressPosition(0);

	return TRUE;
}

BOOL CSyncThread::_ProcessEntry(const u32 &uArgc, char *pArgv[])
{
	static u32 uCnt = 0;

	// NKM - CRC makes it have 5 arguments now
	if (uArgc < 5)
	{
		_SetStatusText("FATAL ERROR: Bad number of arguments to _ProcessEntry()!\n");
		return FALSE;
	}

	u32 uNum = atoi(pArgv[1]);

	if (uNum != m_uNumFileEntries)
	{
		CString sNum;

		sNum.Format("Server %d != Client %d\n", uNum, m_uNumFileEntries);

		_SetStatusText("FATAL ERROR: File entries are out of sync!!!  " + sNum + "\n");
		_Disconnect();

		return FALSE;
	}

	fclib_strcpy(m_FileEntries[uNum].szFilename, pArgv[2]);
	m_FileEntries[uNum].nModifiedTime = (u32) atoi(pArgv[3]);
	m_FileEntries[uNum].nCRC = (u32) atoi(pArgv[4]);

	// Gives a LOT of text
	//_SetStatusText("Got " + CString(m_FileEntries[uNum].szFilename) + " file information...\n");

	++m_uNumFileEntries;
	
	m_pParent->ProgressStep();

	// Don't send okay on last entry
	if (m_uNumFileEntries != m_uExpectedNumFileEntries)
	{
		++uCnt;

		if (uCnt == FSERVER_MAX_COMMANDS)
		{
			uCnt = 0;
			_SendText("ok");
		}
	}

	return TRUE;
}

BOOL CSyncThread::_DetermineNeedUpdateFiles(const u32 &uArgc, char *pArgv[])
{
	m_hSyncState = _SYNC_STATE_NONE;

	CString str;

	str.Format("%d", m_uNumFileEntries);

	_SetStatusText("Finished getting master file information from Xbox!  Got " + str + " files\n");

	if (!_GetFilesNeedUpdate())
	{
		return FALSE;
	}

	if (_MasterFileShouldSend())
	{
		_SetStatusText("Sending entire master file.  Update would take too long\n");
		return _MasterFileSend();
	}

	if (!m_uNumFileEntriesNeedUpdate)
	{
		_SetStatusText("No updates needed!\n");

		// We didn't fail, but we are done looping now
		return FALSE;
	}
	else
	{
		CString sNum;

		sNum.Format("%d", m_uNumFileEntriesNeedUpdate);

		_SetStatusText(sNum + " file(s) must be updated\n\n");
	}

	if (!_SendNeedUpdateFiles())
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL _FindInVector(const std::vector<CString> &rVec, CString Str)
{
	u32 uSize = rVec.size();

	for (u32 i = 0; i < uSize; ++i)
	{
		if (rVec[i] == Str)
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CSyncThread::_GetFilesNeedUpdate(void)
{
	u32 uCount;
	u32 uNum = masterFile.GetNumFileEntries();
	FDataPrjFile_Entry_t *pXboxEntry;
	const FDataPrjFile_Entry_t *pLocalEntry;

	m_uNumFileEntriesNeedUpdate = 0;

	_SetStatusText("\n");

	for (uCount = 0; uCount < uNum; ++uCount)
	{
		pLocalEntry = masterFile.GetFileEntry(uCount);
		pXboxEntry = _GetXboxEntryByName(pLocalEntry->szFilename);

		if (!pXboxEntry)
		{
			_SetStatusText(CString(pLocalEntry->szFilename) + " does not exist on Xbox\n");
			m_FileEntriesNeedUpdate[m_uNumFileEntriesNeedUpdate] = pLocalEntry;
			++m_uNumFileEntriesNeedUpdate;
		}
		else
		{
			BOOL bSend = FALSE;

			if (pLocalEntry->nCRC != 0)
			{
				if (pLocalEntry->nCRC != pXboxEntry->nCRC)
				{
					_SetStatusText(CString(pLocalEntry->szFilename) + " has a different CRC on Xbox\n");
					bSend = TRUE;
				}
			}
			else
			{
				if (pXboxEntry->nModifiedTime == pLocalEntry->nModifiedTime)
				{
					// Check for stuff that we need to force send
					std::vector<CString> &FileList = m_pParent->GetFileList();

					if (!FileList.empty() && _FindInVector(FileList, pLocalEntry->szFilename))
					{
						_SetStatusText(CString(pLocalEntry->szFilename) + " is a file that must be forced sent.\n");
						bSend = TRUE;
					}
				}
				else
				{
					// No timestamp match
					_SetStatusText(CString(pLocalEntry->szFilename) + " has a different time stamp on Xbox\n");
					bSend = TRUE;
				}
			}

			if (bSend)
			{
				m_FileEntriesNeedUpdate[m_uNumFileEntriesNeedUpdate] = pLocalEntry;
				++m_uNumFileEntriesNeedUpdate;
			}
		}
		/*else if (pXboxEntry->nModifiedTime != pLocalEntry->nModifiedTime)
		{
			_SetStatusText(CString(pLocalEntry->szFilename) + " has a different version on Xbox\n");
			m_FileEntriesNeedUpdate[m_uNumFileEntriesNeedUpdate] = pLocalEntry;
			++m_uNumFileEntriesNeedUpdate;
		}
		else if (pXboxEntry->nModifiedTime == pLocalEntry->nModifiedTime || pXboxEntry->nCRC == pLocalEntry->nCRC)
		{
			// Check for stuff that we need to force send
			std::vector<CString> &FileList = m_pParent->GetFileList();

			if (!FileList.empty())
			{
				if (_FindInVector(FileList, pLocalEntry->szFilename))
				{
					_SetStatusText(CString(pLocalEntry->szFilename) + " is being force sent on Xbox\n");
					m_FileEntriesNeedUpdate[m_uNumFileEntriesNeedUpdate] = pLocalEntry;
					++m_uNumFileEntriesNeedUpdate;
				}
			}
		}*/
	}

	m_pParent->SetProgressRange(0, 0);
	m_pParent->SetProgressPosition(0);

	return TRUE;
}

BOOL CSyncThread::_SendNeedUpdateFiles(void)
{
	m_uNumFileEntriesSent = 0;
	m_hSyncState = _SYNC_STATE_SEND_FILES;

	return TRUE;
}

BOOL CSyncThread::_SendNextFile(void)
{
	u32 uCnt = m_uNumFileEntriesSent;
	u8 *puFileData = masterFile.LoadFile(m_FileEntriesNeedUpdate[uCnt]);

	if (!puFileData)
	{
		_SetStatusText("FATAL ERROR: Unable to load " + CString(m_FileEntriesNeedUpdate[uCnt]->szFilename) + " from master file!\n");
		return FALSE;
	}

	FFileHandle hFile = ffile_Open("temp.dat", FFILE_OPEN_TRUNC_OR_CREATE_WONLY, TRUE);

	if (!FFILE_IS_VALID_HANDLE(hFile))
	{
		_SetStatusText("FATAL ERROR: Invalid Fang file handle!\n");
		return FALSE;
	}

	s32 uFileWrite = ffile_Write(hFile, m_FileEntriesNeedUpdate[uCnt]->nNumBytes, puFileData);

	ffile_Close(hFile);

	delete [] puFileData;

	if (uFileWrite != (s32) m_FileEntriesNeedUpdate[uCnt]->nNumBytes)
	{
		_SetStatusText("FATAL ERROR: Disk write error!\n");
		return FALSE;
	}

	CString sFileName = m_FileEntriesNeedUpdate[uCnt]->szFilename;
	CString sDestName = FSERVER_CLIENT_TEMP_DIR + sFileName;

	_SetStatusText("Sending file " + sFileName + " to Xbox...\n");

	HRESULT hResult = DmSendFile("temp.dat", sDestName);

	if (FAILED(hResult))
	{
		_SetStatusText("Failed to send " + CString(m_FileEntriesNeedUpdate[uCnt]->szFilename) + " to Xbox\n");
		return FALSE;
	}

	++m_uNumFileEntriesSent;

	if (m_uNumFileEntriesSent == m_uNumFileEntriesNeedUpdate)
	{
		DeleteFile("temp.dat");

		CString sNumString;

		sNumString.Format("%d", m_uNumFileEntriesNeedUpdate);

		_SetStatusText("Sent " + sNumString + " file(s) to Xbox!\n\n");

		// Send the information to the Xbox about the files we just sent
		m_hSyncState = _SYNC_STATE_SEND_MASTER_INFO;
		m_uNumFileEntriesSent = 0;

		_SetStatusText("Sending updated file timestamps to Xbox...\n");
		
		if (!_SendText("update|%d", m_uNumFileEntriesNeedUpdate))
		{
			return FALSE;
		}

		_SendNextMasterEntry();
	}
	else if (!(m_uNumFileEntriesSent % FSERVER_FILE_SEND_WORK_MSG))
	{
		// Just sent a message to let the server know we are still here
		_SendText("dummy");
	}

	return TRUE;
}

void CSyncThread::_SendNextMasterEntry(void)
{
	const FDataPrjFile_Entry_t *entry = masterFile.GetFileEntry(m_uNumFileEntriesSent);

	_SetStatusText("Sending file information for " + CString(m_FileEntriesNeedUpdate[m_uNumFileEntriesSent]->szFilename) + "...\n");

	_SendText("update_file|%s|%d|%d", 
			m_FileEntriesNeedUpdate[m_uNumFileEntriesSent]->szFilename, 
			m_FileEntriesNeedUpdate[m_uNumFileEntriesSent]->nModifiedTime,
			m_FileEntriesNeedUpdate[m_uNumFileEntriesSent]->nCRC); 

	++m_uNumFileEntriesSent;

	if (m_uNumFileEntriesSent == m_uNumFileEntriesNeedUpdate)
	{
		_SetStatusText("\nPlease wait while Xbox processes data...\n");

		_SendText("update_done");
		
		m_hSyncState = _SYNC_STATE_NONE;

		m_pParent->SetProgressRange(0, m_uNumFileEntriesSent);
		m_pParent->SetProgressStep(FSERVER_FILE_SEND_WORK_MSG);
		m_pParent->SetProgressPosition(0);
	}
}

BOOL CSyncThread::_Done(const u32 &uArgc, char *pArgv[])
{
	if (uArgc != 2)
	{
		return FALSE;
	}

	if ((u32) atoi(pArgv[1]) != m_uNumFileEntriesSent)
	{
		_SetStatusText("\nUpdate failed\n");
	}
	else
	{
		_SetStatusText("\nUpdate done!\n");
	}

	m_pParent->SetProgressRange(0, 0);
	m_pParent->SetProgressPosition(0);

	m_bRunThread = FALSE;

	return FALSE;
}

BOOL CSyncThread::_MasterFileSend(void)
{
	_SendText("master_send");
	_SetStatusText("Sending master file to Xbox.  This will take a while...\n");

	// Sleep so the server has a chance to process the command and close the master file
	Sleep(2000);

	// !!! THIS IS ONLY OKAY SINCE WE DISCONNECT FROM THE SERVER
	masterFile.Close();

	HRESULT hResult = DmSendFile(m_sLocalMasterFile, FSERVER_FULL_MASTER_PATH);

	if (FAILED(hResult))
	{
		_SetStatusText("Failed to send " + m_sLocalMasterFile + " to Xbox\n");
		return FALSE;
	}

	_SetStatusText("Done sending master file to Xbox.\n");
	_SendText("master_end");

	_Disconnect();

	if (m_sLaunchTitle.IsEmpty())
	{
		_SetStatusText("Trying to reboot Xbox to the XDK Launcher...");

		if (FAILED(DmReboot(DMBOOT_WARM)))
		{
			_SetStatusText("\nFailed to reboot Xbox!\n");
			return FALSE;
		}
	}
	
	return TRUE;
}

BOOL CSyncThread::_MasterFileShouldSend(void)
{
	if (!m_uNumFileEntriesNeedUpdate)
		return FALSE;

	const f32 uByteSendRateMaster = 10 * (1024 * 1024); // Assume 10 meg a sec performance.
	const f32 uByteSendRateFiles = 5 * (1024 * 1024); // Assume 5 meg a sec performance. Slower since we have disk work
	f32 uTimeToSendMaster = masterFile.GetFileSize() / uByteSendRateMaster;
	u32 uFileUpdateSize = 0;

	for (u32 uCnt = 0; uCnt < m_uNumFileEntriesNeedUpdate; ++uCnt)
	{
		uFileUpdateSize += m_FileEntriesNeedUpdate[uCnt]->nNumBytes;
	}

	if (uTimeToSendMaster < (uFileUpdateSize / uByteSendRateFiles))
	{
		_SetStatusText("It is faster to send masterfile than updates...");
		return TRUE;
	}

	if (((f32) m_uNumFileEntriesNeedUpdate / masterFile.GetNumFileEntries()) >= 0.25f)
	{
		_SetStatusText("Greater than 25% of files need updating...");
		return TRUE;
	}

	return FALSE;
}

static HANDLE _hReboot;

DWORD CALLBACK XboxNotification(DWORD dwMsg, DWORD dwParam)
{
	switch(dwMsg & DM_NOTIFICATIONMASK)
	{
		case DM_EXEC:
			if(dwParam == DMN_EXEC_PENDING)
			{
				SetEvent(_hReboot);
			}
		break;
	}
	
	return 0;
}

void CSyncThread::_LaunchTitle(void)
{
	CString sGameName = m_sLaunchTitle;
	CString sGamePath = FSERVER_ROOT_DIR;

	HRESULT hr;

	hr = DmSetXboxNameNoRegister(m_sXboxName);

	if (FAILED(hr))
	{
		_SetStatusText("Failed to set Xbox to " + m_sXboxName + "!\n");
		return;
	}

	// This .xbe does not exist, just used to know when to reboot to the XDK launcher
	if (sGameName == "xdk.xbe")
	{
		_SetStatusText("Trying to reboot Xbox to the XDK Launcher...");

		if (FAILED(DmReboot(DMBOOT_WARM)))
		{
			_SetStatusText("\nFailed to reboot Xbox!\n");
			return;
		}

		_SetStatusText("Done!\n");

		return;
	}

	_SetStatusText("Trying to launch " + sGamePath + sGameName + "...");

	// Cut down on repeated connection requests 
	//DmUseSharedConnection(TRUE);

	// Determine if file exists before rebooting
	hr = DmSetTitle(sGamePath, sGameName, "");

	if (FAILED(hr))
	{
		// hr = 82db0002 for no-such-file
		if (0x82db0002 == hr)
		{
			_SetStatusText("\nGame " + sGameName + " does not exist!\n");
			return;
		}
		/*else
			_SetStatusText("\nSet title failed!\n");
		return;*/
	}
	
	_hReboot = CreateEvent(NULL, FALSE, FALSE, NULL);

	PDMN_SESSION pSession;

	if (FAILED(DmOpenNotificationSession(DM_PERSISTENT, &pSession)))
	{
		_SetStatusText("\nFailed to open a notification session with Xbox!\n");
		return;
	}

	if (FAILED(DmNotify(pSession, DM_EXEC, XboxNotification)))
	{
		_SetStatusText("\nFailed to setup notify with Xbox!\n");
		return;
	}

	if (FAILED(DmReboot(DMBOOT_WARM | DMBOOT_WAIT)))
	{
		_SetStatusText("\nFailed to reboot Xbox!\n");
		return;
	}

	if(WaitForSingleObject(_hReboot, 30000) == WAIT_TIMEOUT)
	{
		_SetStatusText("\nConnection lost with Xbox!\n");
		CloseHandle(_hReboot);
		return;
	}

	CloseHandle(_hReboot);
	
	if (FAILED(DmSetTitle(sGamePath, sGameName, "")))
	{
		_SetStatusText("\nSet title failed after reboot!\n");
		return;
	}

	DmGo();
	DmCloseNotificationSession(pSession);
	//DmUseSharedConnection(FALSE);

	_SetStatusText("Launched!\n");
}

void CSyncThread::_ForceRebootServer(void)
{
	CString sGameName = m_sLaunchTitle;

	m_sLaunchTitle = "xserver.xbe";

	_LaunchTitle();

	m_sLaunchTitle = sGameName;
}

FDataPrjFile_Entry_t *CSyncThread::_GetXboxEntryByName(const char *pszName)
{
	FDataPrjFile_Entry_t tempEntry;

	fclib_strcpy(tempEntry.szFilename, (cchar *) pszName);
	return (FDataPrjFile_Entry_t *) bsearch(&tempEntry, m_FileEntries, m_uNumFileEntries, sizeof(FDataPrjFile_Entry_t), _QSortCompareFcn);
}

BOOL CSyncThread::_SendText(const char *pszFmt, ...)
{
	if (!m_XboxConnection)
	{
		_SetStatusText("\nNot connected!\n");
		return FALSE;
	}

	static const u32 _uPrefixLen = strlen(FSERVER_COMMAND_PREFIX);
	char pszBuff[FSERVER_MAX_SEND_BUFFER];
    va_list arglist;

	fclib_strncpy(pszBuff, FSERVER_COMMAND_PREFIX, _uPrefixLen);
	pszBuff[_uPrefixLen] = '!';

    // format arguments
    va_start(arglist, pszFmt);
    _vsnprintf(pszBuff + _uPrefixLen + 1, FSERVER_MAX_SEND_BUFFER - _uPrefixLen - 1, pszFmt, arglist);
    va_end(arglist);

	DWORD cchResp = FSERVER_MAX_SEND_BUFFER;
    char szResp[FSERVER_MAX_SEND_BUFFER];
    HRESULT hr = DmSendCommand(m_XboxConnection, pszBuff, szResp, &cchResp);

	if (FAILED(hr))
	{
		_SetStatusText("\nDmSendCommand() failed!\n");
		return FALSE;
	}

	return !FAILED(hr);
}

void CSyncThread::_SetStatusText(const CString &sStatus)
{
	m_pParent->StatusTextSet(sStatus);
}