//////////////////////////////////////////////////////////////////////////////////////
// SettingsDlg.cpp - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 07/20/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "pasm.h"
#include "SettingsDlg.h"
#include "PickDir.h"
#include "StringInput.h"
#include "ConfigFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog


CSettingsDlg::CSettingsDlg( CWnd* pParent /*=NULL*/ )
	: CDialog( CSettingsDlg::IDD, pParent ) {
	//{{AFX_DATA_INIT(CSettingsDlg)
	m_sXBMasterFileDir = _T("");
	m_sGCMasterFileDir = _T("");
#ifdef _MMI_TARGET_PS2
    m_sPS2MasterFileDir = _T("");
#endif
	m_sLogDir = _T("");
	m_sExtractDir = _T("");
	m_sConfigDir = _T("");
	m_sIgnoreExtensions = _T("");
	m_sConfigFilename = _T("");
	m_sMasterFilename = _T("");
	m_sLibDir = _T("");
	m_sLocalDir = _T("");
	m_sLibraryLockDir = _T("");
	m_sXClientDir = _T("");
	m_bAutoRunXClient = FALSE;
	//}}AFX_DATA_INIT

	m_hAccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDD));
}


void CSettingsDlg::DoDataExchange( CDataExchange* pDX ) {
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CSettingsDlg)
	DDX_Text( pDX, IDC_XB_MASTER_FILE_DIR, m_sXBMasterFileDir );
	DDX_Text( pDX, IDC_GC_MASTER_FILE_DIR, m_sGCMasterFileDir );
#ifdef _MMI_TARGET_PS2
	DDX_Text( pDX, IDC_PS2_MASTER_FILE_DIR, m_sPS2MasterFileDir );
#endif
	DDX_Text( pDX, IDC_LOG_OUTPUT_DIR, m_sLogDir );
	DDX_Text( pDX, IDC_EXTRACT_DIR, m_sExtractDir );
	DDX_Text( pDX, IDC_CONFIG_DIR, m_sConfigDir );
	DDX_Text( pDX, IDC_IGNORE_EXTENSIONS, m_sIgnoreExtensions );
	DDX_Text( pDX, IDC_CONFIG_FILE, m_sConfigFilename );
	DDX_Text( pDX, IDC_MASTER_FILE, m_sMasterFilename );
	DDX_Text( pDX, IDC_LIB_DIR, m_sLibDir );
	DDX_Text( pDX, IDC_LOCAL_DIR, m_sLocalDir );
	DDX_Text(pDX, IDC_LIBRARY_LOCK_DIR, m_sLibraryLockDir);
	DDX_Text(pDX, IDC_XCLIENT_DIR, m_sXClientDir);
	DDX_Check(pDX, IDC_AUTO_RUN_XCLIENT, m_bAutoRunXClient);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP( CSettingsDlg, CDialog ) 
	//{{AFX_MSG_MAP(CSettingsDlg)
	ON_BN_CLICKED( IDC_CHOOSE_XB_MASTER_FILE_DIR, OnChooseXBMasterFileDir )
	ON_BN_CLICKED( IDC_CHOOSE_GC_MASTER_FILE_DIR, OnChooseGCMasterFileDir )
#ifdef _MMI_TARGET_PS2
	ON_BN_CLICKED( IDC_CHOOSE_PS2_MASTER_FILE_DIR, OnChoosePS2MasterFileDir )
#endif
	ON_BN_CLICKED( IDC_CHOOSE_LOG_OUTPUT_DIR, OnChooseLogOutputDir )
	ON_BN_CLICKED( IDC_CHOOSE_EXTRACT_DIR, OnChooseExtractDir )
	ON_BN_CLICKED( IDC_CHOOSE_CONFIG_DIR, OnChooseConfigDir )
	ON_BN_CLICKED( IDC_CREATE_NEW_CONFIG_NAME, OnNewConfigFile )
	ON_BN_CLICKED( IDC_CHOOSE_CONFIG_FILE, OnChooseConfigFile )
	ON_BN_CLICKED( IDC_CREATE_NEW_MASTER_NAME, OnNewMasterFile )
	ON_BN_CLICKED( IDC_CHOOSE_MASTER_FILE, OnChooseMasterFile )
	ON_BN_CLICKED( IDC_CHOOSE_LIB_DIR, OnChooseLibDir )
	ON_BN_CLICKED( IDC_CHOOSE_LOCAL_DIR, OnChooseLocalDir )
	ON_BN_CLICKED(IDC_CHOOSE_LIB_LOCK_DIR, OnChooseLibLockDir)
	ON_BN_CLICKED(IDC_CHOOSE_XCLIENT_DIR, OnChooseXclientDir)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg message handlers

void CSettingsDlg::OnChooseXBMasterFileDir() {
	
	UpdateData( CONTROLS_TO_VARS );
	CString sNewDir;
	cchar *pszIntialDir;
	if( m_sXBMasterFileDir.IsEmpty() ) {
		pszIntialDir = NULL;
	} else {
		pszIntialDir = m_sXBMasterFileDir;
	}

	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select Xbox Master File Directory", pszIntialDir ) == PICKDIR_RET_SUCCESS ) {
		m_sXBMasterFileDir = sNewDir;
		m_sXBMasterFileDir.MakeLower();
		UpdateData( VARS_TO_CONTROLS );
	}
}

void CSettingsDlg::OnChooseGCMasterFileDir() {
	
	UpdateData( CONTROLS_TO_VARS );
	CString sNewDir;
	cchar *pszIntialDir;
	if( m_sGCMasterFileDir.IsEmpty() ) {
		pszIntialDir = NULL;
	} else {
		pszIntialDir = m_sGCMasterFileDir;
	}

	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select GameCube Master File Directory", pszIntialDir ) == PICKDIR_RET_SUCCESS ) {
		m_sGCMasterFileDir = sNewDir;
		m_sGCMasterFileDir.MakeLower();
		UpdateData( VARS_TO_CONTROLS );
	}
}

#ifdef _MMI_TARGET_PS2

void CSettingsDlg::OnChoosePS2MasterFileDir() {

	UpdateData( CONTROLS_TO_VARS );
	CString sNewDir;
	cchar *pszIntialDir;
	if( m_sPS2MasterFileDir.IsEmpty() ) {
		pszIntialDir = NULL;
	} else {
		pszIntialDir = m_sPS2MasterFileDir;
	}

	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select PS2 Master File Directory", pszIntialDir ) == PICKDIR_RET_SUCCESS ) {
		m_sPS2MasterFileDir = sNewDir;
		m_sPS2MasterFileDir.MakeLower();
		UpdateData( VARS_TO_CONTROLS );
	}
}

#endif

void CSettingsDlg::OnChooseLogOutputDir() {
	
	UpdateData( CONTROLS_TO_VARS );
	CString sNewDir;
	cchar *pszIntialDir;
	if( m_sLogDir.IsEmpty() ) {
		pszIntialDir = NULL;
	} else {
		pszIntialDir = m_sLogDir;
	}

	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select Log Directory", pszIntialDir ) == PICKDIR_RET_SUCCESS ) {
		m_sLogDir = sNewDir;
		m_sLogDir.MakeLower();
		UpdateData( VARS_TO_CONTROLS );
	}		
}

void CSettingsDlg::OnChooseExtractDir() {
	
	UpdateData( CONTROLS_TO_VARS );
	CString sNewDir;
	cchar *pszIntialDir;
	if( m_sExtractDir.IsEmpty() ) {
		pszIntialDir = NULL;
	} else {
		pszIntialDir = m_sExtractDir;
	}

	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select Extract Directory", pszIntialDir ) == PICKDIR_RET_SUCCESS ) {
		m_sExtractDir = sNewDir;
		m_sExtractDir.MakeLower();
		UpdateData( VARS_TO_CONTROLS );
	}	
}

void CSettingsDlg::OnChooseConfigDir() {
	
	UpdateData( CONTROLS_TO_VARS );
	CString sNewDir;
	cchar *pszIntialDir;
	if( m_sConfigDir.IsEmpty() ) {
		pszIntialDir = NULL;
	} else {
		pszIntialDir = m_sConfigDir;
	}

	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select Configuration Directory", pszIntialDir ) == PICKDIR_RET_SUCCESS ) {
		m_sConfigDir = sNewDir;
		m_sConfigDir.MakeLower();
		UpdateData( VARS_TO_CONTROLS );
	}	
}

void CSettingsDlg::OnNewConfigFile() {
	UpdateData( CONTROLS_TO_VARS );
	
	CStringInput StringInput;
	CString sNewName;
	CString sDefault = m_sConfigFilename;
	if( sDefault.IsEmpty() ) {
		// assign a better default
		sDefault = "new_config_file.cfg";
	}
	if( StringInput.GetUserString( sDefault, "Enter New Config Filname", "Enter a new configuration filename without a path", sNewName ) ) {
		if( ParseUserEnteredString( sNewName, m_sConfigFilename, TRUE ) ) {
			sNewName += ".cfg";
		}
		m_sConfigFilename = sNewName;
		m_sConfigFilename.MakeLower();
		UpdateData( VARS_TO_CONTROLS );
	}
}

void CSettingsDlg::OnChooseConfigFile() {
	UpdateData( CONTROLS_TO_VARS );
	
	CString str = "";
	if( !m_sConfigDir.IsEmpty() &&
		!m_sConfigFilename.IsEmpty() ) {
		
		CreateFilename( str, m_sConfigDir, m_sConfigFilename );
	}
    CFileDialog dlg( TRUE,
					 ".cfg",
					 str,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 "Config Files (*.cfg)|*.cfg||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select Config File";
	if( dlg.DoModal() == IDOK ) {
		// load the config file
		CConfigFile ConfigFile;
		if( !ConfigFile.Read( dlg.GetPathName() ) ) {
			this->MessageBox( "Trouble reading config file.\nReverting to last entered config file.", 
							  "PASM ERROR",
							  MB_OK | MB_ICONSTOP );
		} else {
			m_sMasterFilename = ConfigFile.m_sMasterFilename;
			m_sLibDir = ConfigFile.m_sLibDir;
			m_sLocalDir = ConfigFile.m_sLocalDir;	
			m_sLibraryLockDir = ConfigFile.m_sLibraryLockDir;
			m_sConfigFilename = dlg.GetFileName();
			m_sConfigFilename.MakeLower();
		}		
		UpdateData( VARS_TO_CONTROLS );
	}	
}

void CSettingsDlg::OnNewMasterFile() {
	UpdateData( CONTROLS_TO_VARS );
	
	CStringInput StringInput;
	CString sNewName;
	CString sDefault = m_sMasterFilename;
	if( sDefault.IsEmpty() ) {
		// assign a better default
		sDefault = "new_master_file";
	}
	if( StringInput.GetUserString( sDefault, "Enter New Master Filname", "Enter a new master filename without extension or path", sNewName ) ) {
		ParseUserEnteredString( sNewName, m_sMasterFilename, TRUE );
		m_sMasterFilename = sNewName;
		m_sMasterFilename.MakeLower();

		// make sure that the new name doesn't have _xb, _gc, or _pc in it, that will be taken care of later
		int nIndex = m_sMasterFilename.Find( "_xb", 0 );
		if( nIndex >= 0 ) {
			m_sMasterFilename.Delete( nIndex, 3 );
		}
		nIndex = m_sMasterFilename.Find( "_gc", 0 );
		if( nIndex >= 0 ) {
			m_sMasterFilename.Delete( nIndex, 3 );
		}
		nIndex = m_sMasterFilename.Find( "_pc", 0 );
		if( nIndex >= 0 ) {
			m_sMasterFilename.Delete( nIndex, 3 );
		}

#ifdef _MMI_TARGET_PS2
		nIndex = m_sMasterFilename.Find( "_ps2", 0 );
		if( nIndex >= 0 ) {
			m_sMasterFilename.Delete( nIndex, 4 );
		}
#endif

		UpdateData( VARS_TO_CONTROLS );
	}
}

void CSettingsDlg::OnChooseMasterFile() {
	UpdateData( CONTROLS_TO_VARS );

	CString str = "";
	if( !m_sXBMasterFileDir.IsEmpty() &&
		!m_sMasterFilename.IsEmpty() ) {
		
		CreateFilename( str, m_sXBMasterFileDir, m_sMasterFilename );
	}
	CFileDialog dlg( TRUE,
					 ".mst",
					 str,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 "Master Files (*.mst)|*.mst||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select Master File";
	if( dlg.DoModal() == IDOK ) {
		m_sMasterFilename = dlg.GetFileTitle();
		m_sMasterFilename.MakeLower();

		// make sure that the new name doesn't have _xb, _gc, or _pc in it, that will be taken care of later
		int nIndex = m_sMasterFilename.Find( "_xb", 0 );
		if( nIndex >= 0 ) {
			m_sMasterFilename.Delete( nIndex, 3 );
		}
		nIndex = m_sMasterFilename.Find( "_gc", 0 );
		if( nIndex >= 0 ) {
			m_sMasterFilename.Delete( nIndex, 3 );
		}
		nIndex = m_sMasterFilename.Find( "_pc", 0 );
		if( nIndex >= 0 ) {
			m_sMasterFilename.Delete( nIndex, 3 );
		}

#ifdef _MMI_TARGET_PS2
		nIndex = m_sMasterFilename.Find( "_ps2", 0 );
		if( nIndex >= 0 ) {
			m_sMasterFilename.Delete( nIndex, 4 );
		}
#endif
		UpdateData( VARS_TO_CONTROLS );
	}			
}

void CSettingsDlg::OnChooseLibDir() {
	
	UpdateData( CONTROLS_TO_VARS );
	CString sNewDir;
	cchar *pszIntialDir;
	if( m_sLibDir.IsEmpty() ) {
		pszIntialDir = NULL;
	} else {
		pszIntialDir = m_sLibDir;
	}

	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select Library Directory", pszIntialDir ) == PICKDIR_RET_SUCCESS ) {
		m_sLibDir = sNewDir;
		m_sLibDir.MakeLower();
		UpdateData( VARS_TO_CONTROLS );
	}	
}

void CSettingsDlg::OnChooseLocalDir() {
	
	UpdateData( CONTROLS_TO_VARS );
	CString sNewDir;
	cchar *pszIntialDir;
	if( m_sLocalDir.IsEmpty() ) {
		pszIntialDir = NULL;
	} else {
		pszIntialDir = m_sLocalDir;
	}

	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select Local Directory", pszIntialDir ) == PICKDIR_RET_SUCCESS ) {
		m_sLocalDir = sNewDir;
		m_sLocalDir.MakeLower();
		UpdateData( VARS_TO_CONTROLS );
	}	
}

void CSettingsDlg::CreateFilename( CString &rDest, const CString &rPath, const CString &rFilename ) {
	rDest = rPath;

	if( rDest.GetAt( rDest.GetLength() - 1 ) != '\\' ) {
		rDest += '\\';
	}
	rDest += rFilename;
}

// returns TRUE if the new string is valid, FALSE if the old string was copied into rUserEnteredString
BOOL CSettingsDlg::ParseUserEnteredString( CString &rUserEnteredString, const CString &rOldString, BOOL bRemoveExtension ) {

	if( rUserEnteredString.IsEmpty() ) {
		rUserEnteredString = rOldString;
		return FALSE;
	}

	CString sTemp = "\\/:?\"<>|";
	int nIndex = rUserEnteredString.FindOneOf( sTemp );
	while( nIndex >= 0 ) {
		rUserEnteredString.Delete( 0, nIndex+1 );
		nIndex = rUserEnteredString.FindOneOf( sTemp );
	}

	if( rUserEnteredString.IsEmpty() ) {
		rUserEnteredString = rOldString;
		return FALSE;
	}

	if( bRemoveExtension ) {
		nIndex = rUserEnteredString.ReverseFind( '.' );
		while( nIndex >= 0 ) {
			rUserEnteredString.Delete( nIndex, rUserEnteredString.GetLength() - nIndex );
			nIndex = rUserEnteredString.ReverseFind( '.' );
		}
		if( rUserEnteredString.IsEmpty() ) {
			rUserEnteredString = rOldString;
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CSettingsDlg::OnInitDialog() 
{
	CDialog ::OnInitDialog();
	
	// TODO: Add extra initialization here
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CSettingsDlg::PreTranslateMessage( MSG* pMsg ) {
	
	if( m_hAccel != NULL ) {
		if( ::TranslateAccelerator( m_hWnd, m_hAccel, pMsg ) ) {
			return TRUE;
		}
	}
	
	return CDialog ::PreTranslateMessage(pMsg);
}

void CSettingsDlg::OnChooseLibLockDir() {

	UpdateData( CONTROLS_TO_VARS );
	CString sNewDir;
	cchar *pszIntialDir;
	if( m_sLibraryLockDir.IsEmpty() ) {
		pszIntialDir = NULL;
	} else {
		pszIntialDir = m_sLibraryLockDir;
	}

	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select Library Lock Directory", pszIntialDir ) == PICKDIR_RET_SUCCESS ) {
		m_sLibraryLockDir = sNewDir;
		m_sLibraryLockDir.MakeLower();
		UpdateData( VARS_TO_CONTROLS );
	}
}

// !!Nate
void CSettingsDlg::OnChooseXclientDir() 
{
	UpdateData( CONTROLS_TO_VARS );

	CString str = m_sXClientDir;
	CFileDialog dlg( TRUE,
					 ".exe",
					 str,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 "Executable files (*.exe)|*.exe||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select XClient File";

	if( dlg.DoModal() == IDOK ) {
		m_sXClientDir = dlg.GetPathName();
		m_sXClientDir.MakeLower();

		UpdateData( VARS_TO_CONTROLS );
	}
}
