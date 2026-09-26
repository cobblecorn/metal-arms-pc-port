// CopyGoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CopyGo.h"
#include "CopyGoDlg.h"

#include "DependencyDialog.h"

#include <stdlib.h>
#include <string.h>

#include "fversion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCopyGoDlg dialog

CCopyGoDlg::CCopyGoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCopyGoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCopyGoDlg)
	m_program_file = _T("");
	m_args = _T("");
	m_wkdir = _T("");
	m_sVersion = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_cr_filename = _T("");
	m_dependency_item = -1;
	m_modified = FALSE;
}

void CCopyGoDlg::Load()
{
	FILE * f = fopen( m_cr_filename, "r" );
	if (f != NULL)
	{
		char line[256];

		m_dependencies.DeleteAllItems();

		int new_id;
		char * p;

		while (fgets( line, sizeof(line), f ))
		{
			while ((p = strrchr(line, '\n')) != NULL)	*p = '\0';
			while ((p = strrchr(line, '\r')) != NULL)	*p = '\0';

			if (strncmp( line, "SRC", 3 ) == 0)
			{
				new_id = m_dependencies.GetItemCount();
				p = &line[3];
				while (*p == ' ') p++;
				m_dependencies.InsertItem( new_id, p );
			}
			else if (strncmp( line, "TGT", 3 ) == 0)
			{
				p = &line[3];
				while (*p == ' ') p++;
				m_dependencies.SetItemText( new_id, 1, p );
			}
			else if (strncmp( line, "PGM", 3 ) == 0)
			{
				p = &line[3];
				while (*p == ' ') p++;
				m_program_file = p;
			}
			else if (strncmp( line, "ARG", 3 ) == 0)
			{
				p = &line[3];
				while (*p == ' ') p++;
				m_args = p;
			}
			else if (strncmp( line, "WKD", 3 ) == 0)
			{
				p = &line[3];
				while (*p == ' ') p++;
				m_wkdir = p;
			}
		}

		fclose( f );
	}
	else
	{
		CString msg = _T("Unable to open ") + m_cr_filename;
		MessageBox( msg, "CopyGo", MB_OK );
	}

	UpdateData( VARS_TO_CONTROLS );

	m_modified = FALSE;
}

void CCopyGoDlg::Save()
{
	UpdateData( CONTROLS_TO_VARS );

	FILE * f = fopen( m_cr_filename, "w" );
	if (f != NULL)
	{
		int item = -1;
		while ((item = m_dependencies.GetNextItem( item, LVNI_ALL )) != -1)
		{
			fprintf( f, "SRC %s\n", m_dependencies.GetItemText( item, 0 ) );
			fprintf( f, "TGT %s\n", m_dependencies.GetItemText( item, 1 ) );
		}

		fprintf( f, "PGM %s\n", m_program_file );
		fprintf( f, "ARG %s\n", m_args );
		fprintf( f, "WKD %s\n", m_wkdir );

		fclose( f );
	}
	else
	{
		CString msg = _T("Unable to open ") + m_cr_filename;
		MessageBox( msg, "CopyGo", MB_OK );
	}

	m_modified = FALSE;
}

void CCopyGoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCopyGoDlg)
	DDX_Control(pDX, IDC_DEPENDENCIES, m_dependencies);
	DDX_Text(pDX, IDC_PROGRAM_FILE, m_program_file);
	DDX_Text(pDX, IDC_ARGS, m_args);
	DDX_Text(pDX, IDC_WKDIR, m_wkdir);
	DDX_Text(pDX, IDC_VERSION, m_sVersion);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCopyGoDlg, CDialog)
	//{{AFX_MSG_MAP(CCopyGoDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_PROGRAM_BROWSE, OnProgramBrowse)
	ON_BN_CLICKED(IDC_ADD, OnAddDependency)
	ON_BN_CLICKED(IDC_REMOVE, OnRemoveDependency)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_DEPENDENCIES, OnItemchangedDependencies)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_EN_CHANGE(IDC_PROGRAM_FILE, OnChangeProgramFile)
	ON_BN_CLICKED(IDC_WKDIR_BROWSE, OnWkdirBrowse)
	ON_EN_CHANGE(IDC_ARGS, OnChangeArgs)
	ON_EN_CHANGE(IDC_WKDIR, OnChangeWkdir)
	ON_NOTIFY(NM_DBLCLK, IDC_DEPENDENCIES, OnDblclkDependencies)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCopyGoDlg message handlers

static UINT auIDToolBar[] = { 
	ID_FILE_NEW,
	ID_FILE_OPEN,
	ID_FILE_SAVE,
	ID_FILE_SAVE_AS,
};

BOOL CCopyGoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Create toolbar at the top of the dialog window
	if (m_toolBar.Create(this))
	{
	    m_toolBar.LoadBitmap(IDB_TOOLBAR);
	    m_toolBar.SetButtons(auIDToolBar, sizeof(auIDToolBar)/sizeof(auIDToolBar[0]));
	}             

	m_toolBar.SetBarStyle(m_toolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);


    // We need to resize the dialog to make room for control bars.
	// First, figure out how big the control bars are.
	CRect rcClientStart;
	CRect rcClientNow;
	GetClientRect(rcClientStart);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 
				   0, reposQuery, rcClientNow);
	
	// Now move all the controls so they are in the same relative
	// position within the remaining client area as they would be
	// with no control bars.
	CPoint ptOffset(rcClientNow.left - rcClientStart.left,
					rcClientNow.top - rcClientStart.top); 

	CRect  rcChild;					
	CWnd* pwndChild = GetWindow(GW_CHILD);
	while (pwndChild)
	{                               
		pwndChild->GetWindowRect(rcChild);
		ScreenToClient(rcChild);
		rcChild.OffsetRect(ptOffset);
		pwndChild->MoveWindow(rcChild, FALSE);
		pwndChild = pwndChild->GetNextWindow();
	}

	// Adjust the dialog window dimensions
	CRect rcWindow;
	GetWindowRect(rcWindow);
	rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
	rcWindow.bottom += rcClientStart.Height() - rcClientNow.Height();
	MoveWindow(rcWindow, FALSE);
	
	// And position the control bars
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
		
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	CRect rect;
	m_dependencies.GetWindowRect( &rect );
	m_dependencies.InsertColumn( 0, "Source",		LVCFMT_LEFT, rect.Width() >> 1,	0 );
	m_dependencies.InsertColumn( 1, "Destination",	LVCFMT_LEFT, rect.Width() >> 1,	0 );

	m_sVersion.Format( "version # %d.%d.%d", fversion_GetToolMajorVer(), 
											 fversion_GetToolMinorVer(),
											 fversion_GetToolSubVer() );

	UpdateData( VARS_TO_CONTROLS );

	if (!m_cr_filename.IsEmpty())
	{
		Load();
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCopyGoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCopyGoDlg::OnPaint() 
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
HCURSOR CCopyGoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

static char BASED_CODE szProgramFilter[] = "Program Files (*.exe)|*.exe|All Files (*.*)|*.*||";

void CCopyGoDlg::OnChangeProgramFile() 
{
	m_modified = TRUE;
}

void CCopyGoDlg::OnProgramBrowse() 
{
	UpdateData( CONTROLS_TO_VARS );

	char init_path[_MAX_PATH];
	char init_drive[_MAX_DRIVE];
	char init_dir[_MAX_DIR];

	if (!m_program_file.IsEmpty())
	{
		_splitpath( m_program_file, init_drive, init_dir, NULL, NULL );
		strcpy( init_path, init_drive );
		strcat( init_path, init_dir );
	}

	CFileDialog	dlg( TRUE, ".exe", "*.exe", OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, szProgramFilter, this );

	if (!m_program_file.IsEmpty())
	{
		dlg.m_ofn.lpstrInitialDir = init_path;
	}

	if (dlg.DoModal() == IDOK)
	{
		m_program_file = dlg.GetPathName();
		m_modified = TRUE;
		UpdateData( VARS_TO_CONTROLS );
	}
}

void CCopyGoDlg::OnWkdirBrowse() 
{
	UpdateData( CONTROLS_TO_VARS );

	char init_path[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];

	if (!m_program_file.IsEmpty())
	{
		_splitpath( m_program_file, drive, dir, NULL, NULL );
		strcpy( init_path, drive );
		strcat( init_path, dir );
	}

	CFileDialog	dlg( TRUE, ".exe", "*.exe", OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_NOVALIDATE, szProgramFilter, this );

	if (!m_program_file.IsEmpty())
	{
		dlg.m_ofn.lpstrInitialDir = init_path;
	}

	if (dlg.DoModal() == IDOK)
	{
		_splitpath( dlg.GetPathName(), drive, dir, NULL, NULL );
		m_wkdir = CString(drive) + CString(dir);
		m_modified = TRUE;
		UpdateData( VARS_TO_CONTROLS );
	}
}

void CCopyGoDlg::OnChangeArgs() 
{
	m_modified = TRUE;
}

void CCopyGoDlg::OnChangeWkdir() 
{
	m_modified = TRUE;
}

void CCopyGoDlg::OnAddDependency() 
{
	CDependencyDialog	dlg;

	if (dlg.DoModal() == IDOK)
	{
		m_modified = TRUE;
		int new_id = m_dependencies.GetItemCount();
		m_dependencies.InsertItem( new_id, dlg.m_source );
		m_dependencies.SetItemText( new_id, 1, dlg.m_destination );
	}
}

void CCopyGoDlg::OnDblclkDependencies(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnEdit();
	
	*pResult = 0;
}

void CCopyGoDlg::OnEdit() 
{
	if (m_dependency_item >= 0)
	{
		CDependencyDialog	dlg;

		dlg.m_source = m_dependencies.GetItemText( m_dependency_item, 0 );
		dlg.m_destination = m_dependencies.GetItemText( m_dependency_item, 1 );

		if (dlg.DoModal() == IDOK)
		{
			m_modified = TRUE;
			m_dependencies.SetItemText( m_dependency_item, 0, dlg.m_source );
			m_dependencies.SetItemText( m_dependency_item, 1, dlg.m_destination );
		}
	}	
}

void CCopyGoDlg::OnRemoveDependency() 
{
	if (m_dependency_item >= 0)
	{
		m_modified = TRUE;
		m_dependencies.DeleteItem( m_dependency_item );
		m_dependency_item = -1;
	}
}

void CCopyGoDlg::OnItemchangedDependencies(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED))
	{
		m_dependency_item = pNMListView->iItem;
	}
	
	*pResult = 0;
}

void CCopyGoDlg::OnOK() 
{
	if (m_modified)
	{
		if (MessageBox( "Save changes before exiting?", "CopyGo", MB_YESNO) == IDYES)
		{
			OnFileSave();
		}
	}
	
	CDialog::OnOK();
}

void CCopyGoDlg::OnCancel() 
{
	if (m_modified)
	{
		if (MessageBox( "Save changes before exiting?", "CopyGo", MB_YESNO) == IDYES)
		{
			OnFileSave();
		}
	}
	
	CDialog::OnCancel();
}

void CCopyGoDlg::OnFileNew() 
{
	if (m_modified)
	{
		if (MessageBox( "Save changes before creating new CopyGo file?", "CopyGo", MB_YESNO) == IDYES)
		{
			OnFileSave();
		}
	}

	m_cr_filename = "";
	m_program_file = "";
	m_dependencies.DeleteAllItems();
	m_modified = FALSE;

	UpdateData( VARS_TO_CONTROLS );
}

static char BASED_CODE szCRFilter[] = "CopyGo Files (*.cr)|*.cr|All Files (*.*)|*.*||";

void CCopyGoDlg::OnFileOpen() 
{
	if (m_modified)
	{
		if (MessageBox( "Save changes before opening new CopyGo file?", "CopyGo", MB_YESNO) == IDYES)
		{
			OnFileSave();
		}
	}

	CFileDialog	dlg( TRUE, ".cr", "*.cr", OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, szCRFilter, this );
	if (dlg.DoModal() == IDOK)
	{
		m_cr_filename = dlg.GetPathName();
		Load();
	}	
}

void CCopyGoDlg::OnFileSave() 
{
	if (m_cr_filename.IsEmpty())
	{
		OnFileSaveAs();
		return;
	}

	Save();
}

void CCopyGoDlg::OnFileSaveAs() 
{
	CFileDialog	dlg( FALSE, ".cr", m_cr_filename, OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, szCRFilter, this );
	if (dlg.DoModal() == IDOK)
	{
		m_cr_filename = dlg.GetPathName();
		Save();
	}	
}
