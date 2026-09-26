// CopyProgressDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CopyGo.h"
#include "CopyProgressDialog.h"

#include <sys/utime.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_COPYCONTINUE		(WM_USER + 1)

/////////////////////////////////////////////////////////////////////////////
// CCopyProgressDialog dialog


CCopyProgressDialog::CCopyProgressDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CCopyProgressDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCopyProgressDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_source_handle = NULL;
	m_target_handle = NULL;	
}


void CCopyProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCopyProgressDialog)
	DDX_Control(pDX, IDC_TARGET, m_target);
	DDX_Control(pDX, IDC_SOURCE, m_source);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCopyProgressDialog, CDialog)
	//{{AFX_MSG_MAP(CCopyProgressDialog)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_COPYCONTINUE, CopyContinue )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCopyProgressDialog message handlers

BOOL CCopyProgressDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_source_handle = fopen( m_source_name, "rb" );
	if (m_source_handle == NULL)
	{
		CString msg = _T("Unable to open file \"") + CString( m_target_name ) + _T( "\" for copy." );
		MessageBox ( msg, "CopyGo", MB_OK );
		m_cancel = TRUE;
		CDialog::OnCancel();
		return TRUE;
	}
	m_target_handle = fopen( m_target_name, "wb" );
	if (m_target_handle == NULL)
	{
		CString msg = _T("Unable to open file \"") + CString( m_target_name ) + _T("\" for copy." );
		MessageBox ( msg, "CopyGo", MB_OK );
		m_cancel = TRUE;
		CDialog::OnCancel();
		return TRUE;
	}

	m_copied = 0;
	m_last_progress = -1;

	m_progress.SetPos( 0 );
	m_cancel = FALSE;

	if (!m_cancel)
	{
		m_timer = SetTimer( 1, 1, NULL );
//		PostMessage( WM_COPYCONTINUE );
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LONG CCopyProgressDialog::CopyContinue( UINT wParam, LONG lParam )
{
	if (!m_cancel)
	{
		if (!feof(m_source_handle))
		{
			size_t read = fread( m_buffer, 1, sizeof(m_buffer), m_source_handle );
			if (!ferror(m_source_handle))
			{
				if (fwrite( m_buffer, 1, read, m_target_handle ) == read)
				{
					m_source.SetWindowText( m_source_name );
					m_target.SetWindowText( m_target_name );
					m_copied += read;
					int new_progress = (int)(100.0f * (float)m_copied / (float)m_size);
					if (new_progress != m_last_progress)
					{
						m_progress.SetPos( new_progress );
						MSG msg;
						while (PeekMessage( &msg, m_hWnd, 0, 0, PM_REMOVE ))
						{
							if (PreTranslateMessage( &msg ) == 0)
							{
								TranslateMessage( &msg );
								DispatchMessage( &msg );
							}
						}
						m_last_progress = new_progress;
					}
					PostMessage( WM_COPYCONTINUE );
				}
				else
				{
					CString msg = _T("Error writing \"") + CString( m_target_name ) + _T("\"");
					MessageBox ( msg, "CopyGo", MB_OK );
					m_cancel = TRUE;
				}
			}
			else
			{
				CString msg = _T("Error reading \"") + CString( m_source_name ) + _T("\"");
				MessageBox ( msg, "CopyGo", MB_OK );
				m_cancel = TRUE;
			}
		}
		else
		{
			fclose( m_source_handle );
			fclose( m_target_handle );
			m_source_handle = NULL;
			m_target_handle = NULL;	
			struct _utimbuf times;
			times.actime = m_atime;
			times.modtime = m_mtime;
			_utime( m_target_name, &times );
			OnOK();
		}
	}
	
	if (m_cancel)
	{
		if (m_source_handle != NULL)
		{
			fclose( m_source_handle );
		}
		if (m_target_handle != NULL)
		{
			fclose( m_target_handle );
		}
		m_source_handle = NULL;
		m_target_handle = NULL;	
		DeleteFile( m_target_name );

		CDialog::OnCancel();
	}

	return 0;
}

void CCopyProgressDialog::OnCancel() 
{
	CString msg = _T("Copy operation aborted, \"") + CString( m_target_name ) + _T("\" deleted." );
	MessageBox ( msg, "CopyGo", MB_OK );
	m_cancel = TRUE;
}

void CCopyProgressDialog::OnOK() 
{
	CDialog::OnOK();
}


void CCopyProgressDialog::OnTimer(UINT nIDEvent) 
{
	KillTimer( m_timer );
	m_timer = 0;
	PostMessage( WM_COPYCONTINUE );	
	CDialog::OnTimer(nIDEvent);
}
