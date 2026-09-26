//////////////////////////////////////////////////////////////////////////////////////
// Settings.cpp - 
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
// 07/19/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "jaws.h"
#include "Settings.h"
#include "pickdir.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define _REGKEY_SETTINGS		JAWS_REG_KEY "\\" "Settings"


/////////////////////////////////////////////////////////////////////////////
// CSettings dialog


CSettings::CSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CSettings::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSettings)
	m_sMasterFile = _T("");
	m_bAutoRunOnLoad = FALSE;
	m_sScreenShotDir = _T("");
	//}}AFX_DATA_INIT

	GetSettingsFromRegistry( &m_CurrentSettings );
	m_sMasterFile = m_CurrentSettings.sMasterFile;
	m_bAutoRunOnLoad = m_CurrentSettings.bAutoRunOnLoad;
	m_sScreenShotDir = m_CurrentSettings.sScreenShotPath;
}

void CSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettings)
	DDX_Text(pDX, IDC_MASTER_FILE, m_sMasterFile);
	DDX_Check(pDX, IDC_RUN_ON_LOAD, m_bAutoRunOnLoad);
	DDX_Text(pDX, IDC_SCREENSHOT_DIR, m_sScreenShotDir);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSettings, CDialog)
	//{{AFX_MSG_MAP(CSettings)
	ON_BN_CLICKED(IDC_BUTTON_MASTER_FILE, OnButtonPickMasterFile)
	ON_BN_CLICKED(IDC_CHOOSE_SCREENSHOT_DIR, OnChooseScreenshotDir)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettings message handlers

BOOL CSettings::OnInitDialog() {
	CDialog::OnInitDialog();

	m_sMasterFile = m_CurrentSettings.sMasterFile;
	m_bAutoRunOnLoad = m_CurrentSettings.bAutoRunOnLoad;
	m_sScreenShotDir = m_CurrentSettings.sScreenShotPath;

	UpdateData( VARS_TO_CONTROLS );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSettings::OnOK() {

	UpdateData( CONTROLS_TO_VARS );

	m_CurrentSettings.sMasterFile = m_sMasterFile;
	m_CurrentSettings.bAutoRunOnLoad = m_bAutoRunOnLoad;
	m_CurrentSettings.sScreenShotPath = m_sScreenShotDir;

	StoreSettingsIntoRegistry( &m_CurrentSettings );
	
	CDialog::OnOK();
}

void CSettings::OnCancel() {
	CDialog::OnCancel();
}

void CSettings::GetCurrentSettings( Settings_t *pDestSettings ) {
	*pDestSettings = m_CurrentSettings;
}

void CSettings::OnButtonPickMasterFile() {

	UpdateData( CONTROLS_TO_VARS );

	CFileDialog dlg( TRUE,
					 ".mst",
					 m_sMasterFile,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 "Master Files (*.mst)|*.mst||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select Master File";
	if( dlg.DoModal() == IDOK ) {
		m_sMasterFile = dlg.GetPathName();
		m_sMasterFile.MakeLower();

		UpdateData( VARS_TO_CONTROLS );
	}
}

void CSettings::OnChooseScreenshotDir() {
	
	UpdateData( CONTROLS_TO_VARS );
	
	CString sNewDir;
	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select Screenshot Directory", m_sScreenShotDir ) == PICKDIR_RET_SUCCESS ) {
		m_sScreenShotDir = sNewDir;
		m_sScreenShotDir.MakeLower();

		UpdateData( VARS_TO_CONTROLS );
	}	
}

void CSettings::GetSettingsFromRegistry( Settings_t *pDestSettings ) {
	long nRetCode;
	HKEY hKey;
	DWORD nBufLength, nValueType, nValue;
	char szStringBuffer[MAX_PATH+1];

	// Set defaults...
	pDestSettings->sMasterFile.Empty();
	pDestSettings->bAutoRunOnLoad = FALSE;
	pDestSettings->sMasterFile.Empty();

	// open the registry 
	nRetCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE, _REGKEY_SETTINGS, 0, KEY_READ, &hKey );
	if( nRetCode != ERROR_SUCCESS ) {
		return;
	}

	// read the master file location from the registry
	nBufLength = MAX_PATH + 1;
	nRetCode = RegQueryValueEx( hKey, "ArtPath", 0, &nValueType, (BYTE *)szStringBuffer, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_SZ ) {
		pDestSettings->sMasterFile = szStringBuffer;
	}

	// read the auto run on load var from the registry
	nBufLength = sizeof( DWORD );
	nRetCode = RegQueryValueEx( hKey, "AutoRunOnLoad", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		pDestSettings->bAutoRunOnLoad = nValue;
	}

	// read the screenshot dir from the registry
	nBufLength = MAX_PATH + 1;
	nRetCode = RegQueryValueEx( hKey, "ScreenShotDir", 0, &nValueType, (BYTE *)szStringBuffer, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_SZ ) {
		pDestSettings->sScreenShotPath = szStringBuffer;
	}

	RegCloseKey( hKey );
}

BOOL CSettings::StoreSettingsIntoRegistry( const Settings_t *pSettings ) {
	long nRetCode;
	DWORD nDisposition;
	HKEY hKey;
	BOOL bSuccess = TRUE;

	// open the registry
	nRetCode = RegCreateKeyEx( HKEY_LOCAL_MACHINE, _REGKEY_SETTINGS, 0, NULL, 0, KEY_WRITE, NULL, &hKey, &nDisposition );
	if( nRetCode != ERROR_SUCCESS ) {
		return FALSE;
	}

	// write the master file location
	nRetCode = RegSetValueEx( hKey, "ArtPath", 0, REG_SZ, (CONST BYTE *)(LPCTSTR)pSettings->sMasterFile, pSettings->sMasterFile.GetLength() + 1 );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	// write the auto run on load var
	nRetCode = RegSetValueEx( hKey, "AutoRunOnLoad", 0, REG_DWORD, (CONST BYTE *)&pSettings->bAutoRunOnLoad, sizeof( DWORD ) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	// write the screenshot dir
	nRetCode = RegSetValueEx( hKey, "ScreenShotDir", 0, REG_SZ, (CONST BYTE *)(LPCTSTR)pSettings->sScreenShotPath, pSettings->sScreenShotPath.GetLength() + 1 );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	RegCloseKey( hKey );

	return bSuccess;
}

