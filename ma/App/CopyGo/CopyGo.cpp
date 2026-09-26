// CopyGo.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CopyGo.h"
#include "CopyGoDlg.h"
#include "CopyProgressDialog.h"
#include "FileInfo.h"

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCopyGoApp

BEGIN_MESSAGE_MAP(CCopyGoApp, CWinApp)
	//{{AFX_MSG_MAP(CCopyGoApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCopyGoApp construction

CCopyGoApp::CCopyGoApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCopyGoApp object

CCopyGoApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCopyGoApp internal functions

void CCopyGoApp::Spawn( const char * cr_filename, const char *pszArg/*=NULL*/ )
{
	CAboutDlg dummy;

	dummy.Create( CAboutDlg::IDD );
	dummy.ShowWindow( SW_HIDE );
	m_pMainWnd = &dummy;

	FILE * f = fopen( cr_filename, "r" );
	if (f != NULL)
	{
		char line[256];
		char * p;
		CString program_file;
		CString args;
		CString wkdir;
		CString source;
		CString target;

		enum {
			SOURCE	= 1,
			TARGET	= 2,
			COPY	= 3,
		};

		int srctgt = 0;

		BOOL	ok = TRUE;

		while (ok && fgets( line, sizeof(line), f ))
		{
			while ((p = strrchr(line, '\n')) != NULL)	*p = '\0';
			while ((p = strrchr(line, '\r')) != NULL)	*p = '\0';

			if (strncmp( line, "SRC", 3 ) == 0)
			{
				p = &line[3];
				while (*p == ' ') p++;
				source = p;
				srctgt |= SOURCE;
			}
			else if (strncmp( line, "TGT", 3 ) == 0)
			{
				p = &line[3];
				while (*p == ' ') p++;
				target = p;
				srctgt |= TARGET;
			}
			else if (strncmp( line, "PGM", 3 ) == 0)
			{
				p = &line[3];
				while (*p == ' ') p++;
				program_file = p;
			}
			else if (strncmp( line, "ARG", 3 ) == 0)
			{
				p = &line[3];
				while (*p == ' ') p++;
				args = p;
			}
			else if (strncmp( line, "WKD", 3 ) == 0)
			{
				p = &line[3];
				while (*p == ' ') p++;
				wkdir = p;
			}

			if( srctgt == COPY ) {
				// allocate our vars
				struct _stat source_stat;
				struct _stat target_stat;
				BOOL copy;
				CCopyProgressDialog dlg;
				
				int nWildCardIndex = source.Find( '*' );
				if( nWildCardIndex < 0 ) {
					// copy a single file
					if( _stat( source, &source_stat ) == 0 ) {
						copy = TRUE;
						if( _stat( target, &target_stat ) == 0 ) {
							if( source_stat.st_mtime <= target_stat.st_mtime ) {
								copy = FALSE;
							}
						} 
						if( copy ) {
							dlg.m_size = source_stat.st_size;
							dlg.m_source_name = source;
							dlg.m_target_name = target;
							dlg.m_atime = source_stat.st_atime;
							dlg.m_ctime = source_stat.st_ctime;
							dlg.m_mtime = source_stat.st_mtime;

							ok = (dlg.DoModal() == IDOK);
						}
					} else {
						CString msg = _T("Unable to access dependency file \"") + source + _T("\"\n")
									+ _T("Probably not found.");
						MessageBox( NULL, msg, "CopyGo", MB_OK|MB_ICONHAND );
						ok = FALSE;
					}
				} else {
					// do a wildcard copy
					CFileInfoArray SourceFiles;
					CFileInfo *pFileInfo;
					const CFileInfo *pFileInfoData;

					CString sDestFile;
					CString sExt = source.Mid( nWildCardIndex );	
					CString sRootDir = source.Mid( 0, nWildCardIndex );

					SourceFiles.AddDir( sRootDir, sExt, FALSE, CFileInfoArray::AP_NOSORT, FALSE );

					int i, nNumFiles;

					nNumFiles = SourceFiles.GetSize();
					pFileInfoData = SourceFiles.GetData();
					for( i=0; i < nNumFiles; i++ ) {
						pFileInfo = (CFileInfo *)&pFileInfoData[i];
						sDestFile = target;
						sDestFile += pFileInfo->GetFileName();

						if( _stat( pFileInfo->GetFilePath(), &source_stat ) == 0 ) {
							copy = TRUE;
							if( _stat( sDestFile, &target_stat ) == 0 ) {
								if( source_stat.st_mtime <= target_stat.st_mtime ) {
									copy = FALSE;
								}
							} 
							if( copy ) {
								dlg.m_size = source_stat.st_size;
								dlg.m_source_name = pFileInfo->GetFilePath();
								dlg.m_target_name = sDestFile;
								dlg.m_atime = source_stat.st_atime;
								dlg.m_ctime = source_stat.st_ctime;
								dlg.m_mtime = source_stat.st_mtime;

								ok = (dlg.DoModal() == IDOK);
							}
						} else {
							CString msg = _T("Unable to access dependency file \"") + pFileInfo->GetFilePath() + _T("\"\n")
										+ _T("Probably not found.");
							MessageBox( NULL, msg, "CopyGo", MB_OK|MB_ICONHAND );
							ok = FALSE;
						}
						if( !ok ) {
							break;
						}
					}
				}

				srctgt = 0;
			}
		}

		fclose( f );

		if( ok && !program_file.IsEmpty() ) {
			CString commandline;

			// CreateProcess
			if( pszArg ) {
				// use the passed in args
				commandline = program_file + " " + pszArg;
			} else {
				// use the args stored in the copyrun file
				commandline = program_file + " " + args;
			}
			STARTUPINFO	startinfo;
			PROCESS_INFORMATION procinfo;
			memset( &startinfo, 0, sizeof(startinfo) );
			if( !CreateProcess( program_file, (char *)(const char *)commandline, NULL, NULL, FALSE, 
				CREATE_NEW_PROCESS_GROUP | DETACHED_PROCESS | NORMAL_PRIORITY_CLASS, 
				NULL, wkdir, &startinfo, &procinfo ) ) {
				LPVOID lpMsgBuf;

				FormatMessage( 
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL 
				);

				CString msg = _T("Program invocation failed:\n\nCommand Line:  ") + commandline + _T("\n\n");
				msg += (const char *)lpMsgBuf;
				// Display the string.
				MessageBox( NULL, msg, "CopyGo", MB_OK|MB_ICONHAND );

				// Free the buffer.
				LocalFree( lpMsgBuf );
 			}
		}
	} else {
		CString msg = _T("Unable to open ") + CString(cr_filename);
		::MessageBox( NULL, msg, "CopyGo", MB_OK );
	}

	dummy.DestroyWindow();
	m_pMainWnd = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CCopyGoApp initialization

BOOL CCopyGoApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CCopyGoDlg dlg;
	m_pMainWnd = &dlg;

	BOOL	spawn = FALSE;
	CString sArgs;
	
	if (m_lpCmdLine[0] != '\0')
	{
		char * p = m_lpCmdLine;
		while (*p != '\0')
		{
			switch (*p)
			{
			case '/': case '-':
				p++;
				switch (*p)
				{					
				case 'r':
				case 'R':
				case 's':
				case 'S':
					spawn = TRUE;
					p++;
					break;
				case 'a' :
				case 'A':
					p++;
					while (*p == ' ') p++;
					while( *p != '\0' && *p != ' ' ) {
						sArgs += *p++;			
					}
					break;
				}
				break;

			default:
				{
					char path[_MAX_PATH];
					char * tgt = path;
					while (*p != '\0')
					{
						*tgt++ = *p++;
					}
					*tgt = '\0';
					dlg.m_cr_filename = path;
				}
				break;
			}
			while (*p == ' ') p++;
		}
	}

	if (spawn)
	{
		Spawn( dlg.m_cr_filename, sArgs );
	}
	else
	{
		int nResponse = dlg.DoModal();
		if (nResponse == IDOK)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with OK
		}
		else if (nResponse == IDCANCEL)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with Cancel
		}
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
