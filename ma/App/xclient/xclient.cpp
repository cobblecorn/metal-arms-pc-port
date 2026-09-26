// xclient.cpp : Defines the class behaviors for the application.
//
#include "fang.h"
#include "stdafx.h"
#include "xclient.h"
#include "xclientDlg.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CCommandLine : public CCommandLineInfo
{
public:
	CCommandLine()
	{
		m_bSync = FALSE;
		m_bLaunchGame = FALSE;
		m_bClose = FALSE;
		m_uState = _STATE_NOTHING;
	}

	BOOL GetSync(void)                     {return m_bSync;}
	BOOL GetLaunchGame(void)			   {return m_bLaunchGame;}
	BOOL GetClose(void)                    {return m_bClose;}
	const CString &GetLaunchGameName(void) {return m_sGameName;}
	const std::vector<CString> &GetFileList(void)         {return m_FileList;}

	// if params to the flags, keep a state of last param
	// STATE_NONE, STATE_SYNC, STATE_LAUNCH etc
	void ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast)
	{
		CString sParam(lpszParam);

		sParam.MakeLower();

		if (bFlag)
		{
			//if (m_uState == _STATE_NOTHING)
			//{
				if (sParam == "sync")
				{
					m_bSync = TRUE;
				}
				else if (sParam == "launch")
				{
					m_bLaunchGame = TRUE;
					m_uState = _STATE_LAUNCH;
				}
				else if (sParam == "close")
				{
					m_bClose = TRUE;
				}
				else if (sParam == "files")
				{
					m_uState = _STATE_FILES;
				}
			//}
		}
		else
		{
			if (m_uState == _STATE_LAUNCH)
			{
				m_sGameName = sParam;
				m_uState = _STATE_NOTHING;
			}
			else if (m_uState == _STATE_FILES) 
			{
				m_FileList.insert(m_FileList.end(), sParam);
			}
			else
				m_uState = _STATE_NOTHING;
		}
	}

private:
	enum
	{
		_STATE_NOTHING = 0,
		_STATE_LAUNCH,
		_STATE_FILES,
	};

	u8 m_uState;
	BOOL m_bSync;
	BOOL m_bLaunchGame;
	BOOL m_bClose;
	CString m_sGameName;
	std::vector<CString> m_FileList;
};

/////////////////////////////////////////////////////////////////////////////
// CXClientApp

BEGIN_MESSAGE_MAP(CXClientApp, CWinApp)
	//{{AFX_MSG_MAP(CXClientApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXClientApp construction

CXClientApp::CXClientApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CXClientApp object

CXClientApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CXClientApp initialization

BOOL CXClientApp::InitInstance()
{
	CCommandLine CmdLine;
    
	ParseCommandLine(CmdLine);

	fang_Init();
		
	if (!fang_Startup())
	{
		// Trouble starting up Fang...
		OutputDebugString( "Could not start up Fang :(\n" );
		return 0;
	}

	AfxEnableControlContainer();
	
	if (!AfxInitRichEdit())
	{
		return 0;
	}

	// Standard initialization

	CXclientDlg dlg;

	dlg.SetAutoSync(CmdLine.GetSync());
	dlg.SetAutoLaunch(CmdLine.GetLaunchGame());
	dlg.SetAutoLaunchGame(CmdLine.GetLaunchGameName());
	dlg.SetAutoClose(CmdLine.GetClose());
	dlg.SetFileList(CmdLine.GetFileList());

	dlg.ConfigLoad("xclient.ini");

	m_pMainWnd = &dlg;

	int nResponse = dlg.DoModal();

	fang_Shutdown();

	dlg.ConfigWrite("xclient.ini");

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
