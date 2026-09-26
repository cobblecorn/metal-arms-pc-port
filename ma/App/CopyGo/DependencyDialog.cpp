// DependencyDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CopyGo.h"
#include "DependencyDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDependencyDialog dialog


CDependencyDialog::CDependencyDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CDependencyDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDependencyDialog)
	m_source = _T("");
	m_destination = _T("");
	//}}AFX_DATA_INIT
}


void CDependencyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDependencyDialog)
	DDX_Text(pDX, IDC_SOURCE, m_source);
	DDX_Text(pDX, IDC_DESTINATION, m_destination);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDependencyDialog, CDialog)
	//{{AFX_MSG_MAP(CDependencyDialog)
	ON_BN_CLICKED(IDC_SOURCE_BROWSE, OnSourceBrowse)
	ON_BN_CLICKED(IDC_DESTINATION_BROWSE, OnDestinationBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDependencyDialog message handlers

static char BASED_CODE szFilter[] = "All Files (*.*)|*.*||";

void CDependencyDialog::OnSourceBrowse() 
{
	UpdateData( TRUE );

	char init_path[_MAX_PATH];
	char init_file[_MAX_PATH];

	char init_drive[_MAX_DRIVE];
	char init_dir[_MAX_DIR];
	char init_leaf[_MAX_FNAME];
	char init_ext[_MAX_EXT];

	if (!m_source.IsEmpty())
	{
		_splitpath( m_source, init_drive, init_dir, init_leaf, init_ext );
		strcpy( init_path, init_drive );
		strcat( init_path, init_dir );
		strcpy( init_file, init_leaf );
		strcat( init_file, init_ext );
	}
	else if (!m_destination.IsEmpty())
	{
		_splitpath( m_destination, init_drive, init_dir, init_leaf, init_ext );
		strcpy( init_path, init_drive );
		strcat( init_path, init_dir );
		strcpy( init_file, init_leaf );
		strcat( init_file, init_ext );
	}
	else
	{
		init_path[0] = '\0';
		init_file[0] = '\0';
	}

	CFileDialog	dlg( TRUE, NULL, init_file, OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_NOVALIDATE, szFilter, this );

	if (init_path[0])
	{
		dlg.m_ofn.lpstrInitialDir = init_path;
	}

	if (dlg.DoModal() == IDOK)
	{
		m_source = dlg.GetPathName();
		UpdateData( FALSE );
	}
}

void CDependencyDialog::OnDestinationBrowse() 
{
	UpdateData( TRUE );

	char init_path[_MAX_PATH];
	char init_file[_MAX_PATH];

	char init_drive[_MAX_DRIVE];
	char init_dir[_MAX_DIR];
	char init_leaf[_MAX_FNAME];
	char init_ext[_MAX_EXT];

	if (!m_destination.IsEmpty())
	{
		_splitpath( m_destination, init_drive, init_dir, init_leaf, init_ext );
		strcpy( init_path, init_drive );
		strcat( init_path, init_dir );
		strcpy( init_file, init_leaf );
		strcat( init_file, init_ext );
	}
	else if (!m_source.IsEmpty())
	{
		_splitpath( m_source, init_drive, init_dir, init_leaf, init_ext );
		strcpy( init_path, init_drive );
		strcat( init_path, init_dir );
		strcpy( init_file, init_leaf );
		strcat( init_file, init_ext );
	}
	else
	{
		init_path[0] = '\0';
		init_file[0] = '\0';
	}

	CFileDialog	dlg( TRUE, NULL, init_file, OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_NOVALIDATE, szFilter, this );

	if (init_path[0])
	{
		dlg.m_ofn.lpstrInitialDir = init_path;
	}

	if (dlg.DoModal() == IDOK)
	{
		m_destination = dlg.GetPathName();
		UpdateData( FALSE );
	}
}

void CDependencyDialog::OnOK() {
	
	UpdateData( TRUE );

	BOOL bSourceEmpty = m_source.IsEmpty();
	BOOL bDestEmpty = m_destination.IsEmpty();

	if( bSourceEmpty ^ bDestEmpty ) {
		// one of the entries is empty, prompt the user
		this->MessageBox( _T("You must specify both a source \nand destination filename."), 
						  _T("CopyGo Error"),
						  MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return;
	}

	if( !bSourceEmpty && !bDestEmpty ) {
		// check for wildcard characters
		int nIndex = m_source.Find( '*' );
		CString s;
		if( nIndex >= 0 ) {
			// make sure the usage is correct
			s = m_source.Mid( nIndex );
			if( nIndex == 0 ||			// the '*' shouldn't be the first character in the source string (there needs to be a path too)
				m_source[nIndex - 1] != '\\' || // the character before the '*' should be a backslash
				s.GetLength() <= 2 ||	// s must be longer than
				s[1] != '.' ||			// a '.' should follow the wildcard character
				s.FindOneOf( "\\/:;?<>|""'" ) >= 0 ) {
				this->MessageBox( _T("There is an error with the source wildcard\nentry.  The correct usage is either:\n*.*  or *.extension\nPlease correct the entry."), 
								  _T("CopyGo Error"), 
								  MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
				return;
			}
			// make sure that the destination is a path name
			if( m_destination[ m_destination.GetLength() - 1 ] != '\\' ) {
				this->MessageBox( _T("The source file contains the wildcard ( * )\ncharacter, so the destination must be a\npath string ending with a ( \\ )"), 
								  _T("CopyGo Error"),
								  MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
				return;
			}
		}
	}
	
	CDialog::OnOK();
}
