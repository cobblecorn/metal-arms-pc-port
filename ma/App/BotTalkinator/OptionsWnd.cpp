// OptionsWnd.cpp : implementation file
//

#include "stdafx.h"
#include "bottalkinator.h"
#include "OptionsWnd.h"
#include "PickDir.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsWnd dialog


COptionsWnd::COptionsWnd(CWnd* pParent /*=NULL*/)
	: CDialog(COptionsWnd::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptionsWnd)
	m_sInputDir = _T("");
	m_sMasterFile = _T("");
	m_sOutputDir = _T("");
	//}}AFX_DATA_INIT
}


void COptionsWnd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsWnd)
	DDX_Text(pDX, IDC_EDIT_INPUT_DIR, m_sInputDir);
	DDX_Text(pDX, IDC_EDIT_MASTERFILE, m_sMasterFile);
	DDX_Text(pDX, IDC_EDIT_OUTPUT_DIR, m_sOutputDir);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsWnd, CDialog)
	//{{AFX_MSG_MAP(COptionsWnd)
	ON_BN_CLICKED(IDC_BUTTON_VIDEOMODE, OnButtonVideomode)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_INPUT_DIR, OnButtonBrowseInputDir)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_OUTPUT_DIR, OnButtonBrowseOutputDir)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEMASTER, OnButtonBrowsemaster)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsWnd message handlers

BOOL COptionsWnd::DoGetOptions( CBotTalkinatorDlg *pParentDlg ) {
	m_pParentDlg = pParentDlg;

	int nResponse = DoModal();
	if( nResponse == IDOK ) {
		// fill in the parent dialog's vars and see if there were any changes made
		BOOL bChanges = FALSE;

		if( m_bValidVideoMode ) {
			if( m_oVidMode.GetVidMode( &m_pParentDlg->m_oGameLoopData.oVidSettings ) ) {
				m_pParentDlg->m_bValidVideoMode = TRUE;
			}
			bChanges = TRUE;
		}

		CSettings &rSettings = CSettings::GetCurrent();

		if( m_bInputDirChanged ) {
			m_pParentDlg->m_sAnimFile = rSettings.m_sInputDir + BOT_TALK_GESTURES_FILENAME;
			m_pParentDlg->m_sDialogueFile = rSettings.m_sInputDir + BOT_TALK_DIALOGUES_FILENAME;
			m_pParentDlg->m_sSoundFile = rSettings.m_sInputDir + BOT_TALK_VOCAB_FILENAME;

			bChanges = TRUE;
		}

		if( m_bOutputDirChanged ) {
			bChanges = TRUE;
		}

		if( m_bMasterFileChanged ) {
			m_pParentDlg->m_sMasterFile = rSettings.m_sMasterFile;

			bChanges = TRUE;
		}

		if( bChanges ) {
			return TRUE;
		}
	}

	// we'll only return true if we selected 'ok' and there were changes made
	return FALSE;
}

void COptionsWnd::OnButtonVideomode() {
	int nResponse = m_oVidMode.DoModal();

	if( nResponse == IDOK ) {
		m_bValidVideoMode = TRUE;
	}
}

void COptionsWnd::OnOK() {

	UpdateData( CONTROLS_TO_VARS );
	
	// update and save out the settings file
	CSettings &rSettings = CSettings::GetCurrent();

	if( !m_bInputDirChanged ) {
		// test for a typed in change of the input directory
		if( rSettings.m_sInputDir.CompareNoCase( m_sInputDir ) != 0 ) {
			m_bInputDirChanged = TRUE;
		}
	}
	rSettings.m_sInputDir = m_sInputDir;
	
	if( !m_bOutputDirChanged ) {
		// test for a typed in change of the output directory
		if( rSettings.m_sOutputDir.CompareNoCase( m_sOutputDir ) != 0 ) {
			m_bOutputDirChanged = TRUE;
		}
	}
	rSettings.m_sOutputDir = m_sOutputDir;

	if( !m_bMasterFileChanged ) {
		// test for a typed in change of the master file location
		if( rSettings.m_sMasterFile.CompareNoCase( m_sMasterFile ) != 0 ) {
			m_bMasterFileChanged = TRUE;
		}
	}
	rSettings.m_sMasterFile = m_sMasterFile;

	rSettings.SaveCommonDataOutToFile();
	
	CDialog::OnOK();
}

BOOL COptionsWnd::OnInitDialog() {

	CDialog::OnInitDialog();

	// track the changes
	m_bValidVideoMode = FALSE;
	m_bMasterFileChanged = FALSE;
	m_bInputDirChanged = FALSE;
	m_bOutputDirChanged = FALSE;

	CSettings &rSettings = CSettings::GetCurrent();

	m_sInputDir = rSettings.GetInputDir();
	m_sMasterFile = rSettings.GetMasterFile();
	m_sOutputDir = rSettings.GetOutputDir();
	
	UpdateData( VARS_TO_CONTROLS );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsWnd::OnButtonBrowseInputDir() {
	
	UpdateData( CONTROLS_TO_VARS );

	CString sNew;
	if( pickdir_PickDir( this->m_hWnd, &sNew, "Select Input File Directory", m_sInputDir ) == PICKDIR_RET_SUCCESS ) {

		if( sNew.CompareNoCase( m_sInputDir ) != 0 ) {
			// the input dir changed
			m_sInputDir = sNew;

			UpdateData( VARS_TO_CONTROLS );

			m_bInputDirChanged = TRUE;
		}
	}
	
}

void COptionsWnd::OnButtonBrowseOutputDir() {
	
	UpdateData( CONTROLS_TO_VARS );

	CString sNew;
	if( pickdir_PickDir( this->m_hWnd, &sNew, "Select Output File Directory", m_sOutputDir ) == PICKDIR_RET_SUCCESS ) {

		if( sNew.CompareNoCase( m_sOutputDir ) != 0 ) {
			// the output dir changed
			m_sOutputDir = sNew;

			UpdateData( VARS_TO_CONTROLS );

			m_bOutputDirChanged = TRUE;
		}
	}	
}

void COptionsWnd::OnButtonBrowsemaster() {
	
	UpdateData( CONTROLS_TO_VARS );

	CFileDialog dlg( TRUE,
					 ".mst",
					 m_sMasterFile,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 "Master Files (*.mst)|*.mst||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select Master File";
	if( dlg.DoModal() == IDOK ) {

		if( m_sMasterFile.CompareNoCase( dlg.GetPathName() ) != 0 ) {
			// the masterfile changed
			m_sMasterFile = dlg.GetPathName();

			UpdateData( VARS_TO_CONTROLS );

			m_bMasterFileChanged = TRUE;
		}
	}	
}
