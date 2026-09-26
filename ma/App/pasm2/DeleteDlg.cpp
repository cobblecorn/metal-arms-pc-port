//////////////////////////////////////////////////////////////////////////////////////
// DeleteDlg.cpp - 
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
#include "DeleteDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDeleteDlg dialog


CDeleteDlg::CDeleteDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDeleteDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDeleteDlg)
	m_bDeleteFromLib = FALSE;
	m_bDeleteFromLoc = FALSE;
	m_bDeleteFromMaster = FALSE;
	m_sFilename = _T("");
	//}}AFX_DATA_INIT

	m_pLibFileInfo = NULL;
	m_pLocFileInfo = NULL;
	m_pszEntryName = NULL;
	m_bLibStartCondition = FALSE;
	m_bLocStartCondition = FALSE;
	m_bMasterStartCondition = FALSE;

	m_bDeleteLibFile = FALSE;
	m_bDeleteLocFile = FALSE;
	m_bDeleteMasterFile = FALSE;

	m_bDeleteAll = FALSE;
}


void CDeleteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeleteDlg)
	DDX_Check(pDX, IDC_FROM_LIB, m_bDeleteFromLib);
	DDX_Check(pDX, IDC_FROM_LOCAL, m_bDeleteFromLoc);
	DDX_Check(pDX, IDC_FROM_MASTER, m_bDeleteFromMaster);
	DDX_Text(pDX, IDC_FILENAME, m_sFilename);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDeleteDlg, CDialog)
	//{{AFX_MSG_MAP(CDeleteDlg)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_DELETE_ALL, OnDeleteAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeleteDlg message handlers

BOOL CDeleteDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	BOOL bFilenameSet = FALSE;

	m_bDeleteFromLib = m_bLibStartCondition;
	if( !m_pLibFileInfo ) {
		m_bDeleteFromLib = FALSE;
		GetDlgItem( IDC_FROM_LIB )->EnableWindow( FALSE );
	} else if ( !bFilenameSet ) {
		m_sFilename = m_pLibFileInfo->GetFileName();
		bFilenameSet = TRUE;
	}

	m_bDeleteFromLoc = m_bLocStartCondition;
	if( !m_pLocFileInfo ) {
		m_bDeleteFromLoc = FALSE;
		GetDlgItem( IDC_FROM_LOCAL )->EnableWindow( FALSE );
	} else if( !bFilenameSet ) {
		m_sFilename = m_pLocFileInfo->GetFileName();
		bFilenameSet = TRUE;
	}

	m_bDeleteFromMaster = m_bMasterStartCondition;
	if( !m_pszEntryName ) {
		m_bDeleteFromMaster = FALSE;
		GetDlgItem( IDC_FROM_MASTER )->EnableWindow( FALSE );
	} else if( !bFilenameSet ) {
		m_sFilename = m_pszEntryName;
		bFilenameSet = TRUE;
	}
	m_sFilename.MakeUpper();
	
	// output fields
	m_bDeleteAll = FALSE;
	m_nFilesToDelete = 0;
	m_bDeleteLibFile = FALSE;
	m_bDeleteLocFile = FALSE;
	m_bDeleteMasterFile = FALSE;

	UpdateData( VARS_TO_CONTROLS );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDeleteDlg::OnDelete() {
	CString sError;
	
	UpdateData( CONTROLS_TO_VARS );

	// we must select something to delete from
	if( !m_bDeleteFromLib && !m_bDeleteFromLoc && !m_bDeleteFromMaster ) {
		sError.Format( "You must check the\ndesired location(s) to delete:\n%s", m_sFilename );
		this->MessageBox( sError, _T("PASM ERROR"), 
						  MB_ICONEXCLAMATION | MB_OK | MB_DEFBUTTON1 );	
		return;
	}

	if( m_bDeleteFromLib ) {
		// delete the library file
		m_bDeleteLibFile = TRUE;
		m_nFilesToDelete++;
	}

	if( m_bDeleteFromLoc ) {
		// delete the local file
		m_bDeleteLocFile = TRUE;
		m_nFilesToDelete++;
	}

	if( m_bDeleteFromMaster ) {
		// delete the master file entry
		m_bDeleteMasterFile = TRUE;
		m_nFilesToDelete++;
	}

	// simulate an ok message
	this->PostMessage( WM_COMMAND, IDOK, 0 );
}

int CDeleteDlg::DoModal() {
		
	return CDialog::DoModal();
}

BOOL CDeleteDlg::PreTranslateMessage( MSG* pMsg ) {

	if( pMsg->message == WM_KEYDOWN ) {
		if( pMsg->wParam == VK_ESCAPE ) {
			// Esc key should not kill the app, do nothing
			return TRUE;
		}
	}

	return CDialog::PreTranslateMessage( pMsg );
}

void CDeleteDlg::OnDeleteAll() {
	
	UpdateData( CONTROLS_TO_VARS );
		
	m_bDeleteAll = TRUE;
	OnDelete();	
}

