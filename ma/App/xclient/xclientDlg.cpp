// xclientDlg.cpp : implementation file
//
#include "fang.h"
#include "stdafx.h"
#include "xclient.h"
#include "xclientDlg.h"
#include "SettingsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXclientDlg dialog

CXclientDlg::CXclientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CXclientDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CXclientDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bSyncChecked = m_bLaunchChecked = FALSE;
	m_sCurrentSel = -1;

	m_bAutoSync = FALSE;
	m_bAutoLaunch = FALSE;
	m_bAutoClose = FALSE;
}

void CXclientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_QUIT, m_QuitButton);
	//{{AFX_DATA_MAP(CXclientDlg)
	DDX_Control(pDX, IDC_BUTTON_QUIT, m_QuitButton);
	DDX_Control(pDX, IDC_PROGRESS1, m_ProgressBar);
	DDX_Control(pDX, IDC_TEXT_STATUS, m_TextStatus);
	DDX_Control(pDX, IDC_COMBO_GAMES, m_GamesCombo);
	DDX_Control(pDX, IDC_CHECK_SYNCMASTER, m_SyncCheck);
	DDX_Control(pDX, IDC_CHECK_LAUNCHGAME, m_LaunchCheck);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CXclientDlg, CDialog)
	//{{AFX_MSG_MAP(CXclientDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SETTINGS, OnButtonSettings)
	ON_BN_CLICKED(IDC_BUTTON_GO, OnButtonGo)
	ON_BN_CLICKED(IDC_BUTTON_QUIT, OnButtonQuit)
	ON_BN_CLICKED(IDC_CHECK_LAUNCHGAME, OnCheckLaunchgame)
	ON_BN_CLICKED(IDC_CHECK_SYNCMASTER, OnCheckSyncmaster)
	ON_CBN_SELCHANGE(IDC_COMBO_GAMES, OnSelchangeComboGames)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CXclientDlg::ConfigLoad(const CString &sFile)
{
	char szTempString[_MAX_PATH];

	GetPrivateProfileString("General", "XboxName", "xbox", szTempString, _MAX_PATH, sFile);
	m_sXboxName = szTempString;

	GetPrivateProfileString("General", "UserName", "ApeDApe", szTempString, _MAX_PATH, sFile);
	m_sUserName = szTempString;

	GetPrivateProfileString("General", "MasterFile", "c:\\art\\master\\mettlearms_xb.mst", szTempString, _MAX_PATH, sFile);
	m_sLocalMasterFile = szTempString;

	GetPrivateProfileString("General", "GameFile", "h:\\tools\\ape_tools\\games.agl", szTempString, _MAX_PATH, sFile);
	m_sGameFile = szTempString;


	GetPrivateProfileString("General", "SyncChecked", "0", szTempString, _MAX_PATH, sFile);
	m_bSyncChecked = atoi(szTempString) != 0 ? true : false;

	GetPrivateProfileString("General", "LaunchChecked", "0", szTempString, _MAX_PATH, sFile);
	m_bLaunchChecked = atoi(szTempString) != 0 ? true : false;

	GetPrivateProfileString("General", "GameSelect", "-1", szTempString, _MAX_PATH, sFile);
	m_sCurrentSel = atoi(szTempString);

	_LoadGameFile();

	if (m_sUserName.GetLength() >= FSERVER_MAX_CLIENT_NAME)
		m_sUserName = m_sUserName.Left(FSERVER_MAX_CLIENT_NAME - 1);
}

void CXclientDlg::ConfigWrite(const CString &sFile)
{
	char szTemp[34];

	WritePrivateProfileString("General", "XboxName", m_sXboxName, sFile);
	WritePrivateProfileString("General", "UserName", m_sUserName, sFile);
	WritePrivateProfileString("General", "MasterFile", m_sLocalMasterFile, sFile);
	WritePrivateProfileString("General", "GameFile", m_sGameFile, sFile);
	WritePrivateProfileString("General", "LaunchChecked", m_bLaunchChecked ? "1" : "0", sFile);
	WritePrivateProfileString("General", "SyncChecked", m_bSyncChecked ? "1" : "0", sFile);
	
	itoa(m_sCurrentSel, szTemp, 10);

	WritePrivateProfileString("General", "GameSelect", szTemp, sFile);
}

/////////////////////////////////////////////////////////////////////////////
// CXclientDlg message handlers

BOOL CXclientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	CString sVer;

	sVer.Format("Ver %d.%d.%d", fversion_GetToolMajorVer(), fversion_GetToolMinorVer(), fversion_GetToolSubVer());

	SetWindowText("SAS Master File Update Client - " + sVer);

	m_SyncThread.SetParent(this);

	_UpdateGameComboBox();

	if (m_bAutoLaunch)
	{
		if (m_sAutoLaunchGame.IsEmpty())
		{
			MessageBox("You must specifcy a launch title with the -launch flag!", "Error", MB_ICONERROR);
			m_bAutoLaunch = FALSE;
		}
	}

	if (m_bLaunchChecked)
	{
		m_LaunchCheck.SetCheck(BST_CHECKED);
	}
	else
	{
		m_LaunchCheck.SetCheck(BST_UNCHECKED);
	}
	
	if (m_bSyncChecked)
	{
		m_SyncCheck.SetCheck(BST_CHECKED);
	}
	else
	{
		m_SyncCheck.SetCheck(BST_UNCHECKED);
	}

	if (m_bAutoSync || m_bAutoLaunch)
	{
		OnButtonGo();
	}

	//m_QuitButton.EnableWindow(FALSE);
   
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CXclientDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CXclientDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CXclientDlg::OnButtonSettings() 
{
	// TODO: Add your control notification handler code here
	CSettingsDialog settings;
	
	settings.SetXboxName(m_sXboxName);
	settings.SetUserName(m_sUserName);
	settings.SetLocalMasterFile(m_sLocalMasterFile);
	settings.SetGameFile(m_sGameFile);
	settings.DoModal();

	m_sXboxName = settings.GetXboxName();
	m_sUserName = settings.GetUserName();
	m_sLocalMasterFile = settings.GetLocalMasterFile();
	
	if (m_sGameFile != settings.GetGameFile())
	{
		m_sGameFile = settings.GetGameFile();

		m_GameList.clear();

		_LoadGameFile();
		_UpdateGameComboBox();
	}
}

void CXclientDlg::OnButtonGo() 
{
	if (m_SyncThread.ThreadRunning())
	{
		MessageBox("Sync process is already running!", "Not finished!", MB_ICONEXCLAMATION);
		return;
	}

	BOOL bSync   = FALSE;
	BOOL bLaunch = FALSE;
	BOOL bClose  = FALSE;
	CString sGame;

	if (m_bAutoSync || m_bAutoLaunch || m_bAutoClose)
	{
		bSync = m_bAutoSync;
		bLaunch = m_bAutoLaunch;
		bClose = m_bAutoClose;
		sGame = m_sAutoLaunchGame;
	}
	else
	{
		bSync = m_bSyncChecked;
		bLaunch = m_bLaunchChecked;

		if (m_bLaunchChecked)
			sGame = _GetLaunchTitle();
	}

	if (bSync || bLaunch)
	{
		m_SyncThread.Reset();
		m_SyncThread.SetParent(this);
		m_SyncThread.SetXboxName(m_sXboxName);
		m_SyncThread.SetUserName(m_sUserName);
		m_SyncThread.SetLocalMasterFile(m_sLocalMasterFile);
		m_SyncThread.SetCloseOnFinish(bClose);
		m_SyncThread.SetLaunchTitle(sGame);

		if (!bSync)
			m_SyncThread.SetLaunchOnly(TRUE);

		m_pWorkThread = AfxBeginThread(CSyncThread::_ThreadFunc, &m_SyncThread, THREAD_PRIORITY_NORMAL, 0, 0);

		m_bAutoSync = m_bAutoLaunch = m_bAutoClose = FALSE;
		m_sAutoLaunchGame = "";
	}
}

void CXclientDlg::OnButtonQuit() 
{
	
	while (m_SyncThread.ThreadRunning())
	{
		m_SyncThread.StopThread();

		// Sleep a bit so our thread can die out
		Sleep(250);
	}

	CDialog::OnOK();	
}

BOOL CXclientDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		{
			// Esc key should not kill the app, do nothing
			return TRUE;
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CXclientDlg::StatusTextSet(const CString &sStatus)
{
	CHARRANGE cr = {-1, -2};
	m_TextStatus.SendMessage(EM_EXSETSEL, 0, (LPARAM) &cr);
    m_TextStatus.SendMessage(EM_REPLACESEL, 0, (LONG) (LPCTSTR) sStatus);
    m_TextStatus.SendMessage(EM_SCROLLCARET, 0, 0L);
}

void CXclientDlg::StatusTextClear(void)
{
	m_TextStatus.SetSel(0, -1);
	m_TextStatus.Clear();
}

void CXclientDlg::_LoadGameFile(void)
{
	FILE *pFile = fopen(m_sGameFile, "r");

	if (!pFile)
		return;

	char szBuffer[1024];

	while (!feof(pFile))
	{
		if (!fgets(szBuffer, 1024, pFile))
		{
			break;
		}

		if (!szBuffer[0])
		{
			continue;
		}

		CString sGameName(szBuffer);

		sGameName.TrimLeft();
		sGameName.TrimRight();

		if (sGameName.IsEmpty())
		{
			continue;
		}

		m_GameList.insert(m_GameList.end(), sGameName);
	}

	fclose(pFile);
}

void CXclientDlg::_UpdateGameComboBox(void)
{
	m_GamesCombo.ResetContent();

	u32 uSize = m_GameList.size();

	for (u32 uCnt = 0; uCnt < uSize; ++uCnt)
	{
		CString sTemp = m_GameList[uCnt];
		int sSemiNdx = sTemp.Find(';');

		int sNdx = m_GamesCombo.AddString(sTemp.Left(sSemiNdx));
		m_GamesCombo.SetItemData(sNdx, sNdx);
	}

	m_GamesCombo.SetCurSel(m_sCurrentSel);
}

CString CXclientDlg::_GetLaunchTitle(void)
{
	// With auto launch use the name we were given
	if (m_bAutoLaunch)
	{
		return m_sAutoLaunchGame;
	}

	int sSel = m_GamesCombo.GetCurSel();

	if (sSel == -1)
	{
		MessageBox("Select a game to launch!", "Error", MB_ICONERROR);
		return CString();
	}

	DWORD itemData = m_GamesCombo.GetItemData(sSel);
	CString sSelected = m_GameList[itemData];
	int sFound = sSelected.ReverseFind(';');

	return sSelected.Right(sSelected.GetLength() - sFound - 1);
}

void CXclientDlg::OnCheckLaunchgame() 
{
	m_bLaunchChecked = !m_bLaunchChecked;
}

void CXclientDlg::OnCheckSyncmaster() 
{
	m_bSyncChecked = !m_bSyncChecked;
}


void CXclientDlg::OnSelchangeComboGames() 
{
	m_sCurrentSel = m_GamesCombo.GetCurSel();
}

void CXclientDlg::OnClose() 
{
	while (m_SyncThread.ThreadRunning())
	{
		m_SyncThread.StopThread();

		// Sleep a bit so our thread can die out
		Sleep(250);
	}	
	CDialog::OnClose();
}
