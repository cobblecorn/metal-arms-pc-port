// SyncDialog.cpp : implementation file
//
#include "fang.h"
#include "stdafx.h"
#include "pasm.h"
#include "SyncDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSyncDialog dialog


CSyncDialog::CSyncDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSyncDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSyncDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pWorkThread = 0;
}


void CSyncDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSyncDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSyncDialog, CDialog)
	//{{AFX_MSG_MAP(CSyncDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSyncDialog message handlers

int CSyncDialog::DoModal() 
{
	m_pWorkThread = AfxBeginThread( CSyncDialog::WorkerThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED );	

	return CDialog::DoModal();
}

UINT CSyncDialog::WorkerThread( LPVOID pParam )
{
	CSyncDialog *pDlg = (CSyncDialog *) pParam;

	FASSERT(pDlg);

	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    
	si.cb = sizeof(si);

	CString sCommandLineString = pDlg->m_sXClientFileDir + " -sync -launch xdk.xbe -close";
	
	if( pDlg->m_FilesToForceCompile.GetSize() > 0 ) {
		CString sForceFileString = "-files ";

		int nNumFiles = pDlg->m_FilesToForceCompile.GetSize();
		const CFileInfo *pFileInfo;
		CString sLowerFileName;

		for( int i = 0; i < nNumFiles; ++i ) {
			pFileInfo = &pDlg->m_FilesToForceCompile[i];

			sLowerFileName = pFileInfo->GetFileName();
			sLowerFileName.MakeLower();

			sForceFileString.Append( sLowerFileName + " " );
		}

		sCommandLineString.Append( " " + sForceFileString );
	}


    // Start the child process. 
    if( !CreateProcess( NULL, // No module name (use command line). 
		(char *) (cchar *) (sCommandLineString), 
        NULL,             // Process handle not inheritable. 
        NULL,             // Thread handle not inheritable. 
        FALSE,            // Set handle inheritance to FALSE. 
        0,                // No creation flags. 
        NULL,             // Use parent's environment block. 
        NULL,             // Use parent's starting directory. 
        &si,              // Pointer to STARTUPINFO structure.
        &pi )             // Pointer to PROCESS_INFORMATION structure.
    ) 
    {
        pDlg->MessageBox("Failed to launch " + pDlg->m_sXClientFileDir + "!", "Error", MB_ICONEXCLAMATION | MB_OK);
		pDlg->OnOK();

		return 0;
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

	pDlg->PostMessage(WM_COMMAND, IDOK, 0);

	return 0;
}

BOOL CSyncDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_pWorkThread->ResumeThread();	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
