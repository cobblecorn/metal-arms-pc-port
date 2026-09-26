//////////////////////////////////////////////////////////////////////////////////////
// StringInput.cpp - general class for getting a users string input
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
// 05/31/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#ifdef _MMI_TARGET_PS2
#include "ps2\resource.h"
#else
#include "resource.h"
#endif
#include "StringInput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define _VARS_TO_CONTROLS	FALSE
#define _CONTROLS_TO_VARS	TRUE

/////////////////////////////////////////////////////////////////////////////
// CStringInput dialog

CStringInput::CStringInput( CWnd* pParent /*=NULL*/ )
	: CDialog( CStringInput::IDD, pParent )
{
	//{{AFX_DATA_INIT(CStringInput)
	m_sInstructions = _T("");
	m_sUserString = _T("");
	//}}AFX_DATA_INIT
}


void CStringInput::DoDataExchange( CDataExchange* pDX ) {
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CStringInput)
	DDX_Text( pDX, IDC_STATIC_INSTRUCTIONS, m_sInstructions );
	DDX_Text( pDX, IDC_INPUT_USER_STRING, m_sUserString );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStringInput, CDialog)
	//{{AFX_MSG_MAP(CStringInput)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CStringInput::OnInitDialog() {
	CDialog::OnInitDialog();
	SetWindowText( m_sTitle );
	CenterWindow();

	UpdateData( _VARS_TO_CONTROLS );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////
// CStringInput message handlers

// returns TRUE if a string was entered, FALSE if not
BOOL CStringInput::GetUserString( cchar *pszDefault, cchar *pszTitle, cchar *pszInstructions, CString& sReturn ) {
	
	m_sTitle = pszTitle;
	m_sInstructions = pszInstructions;
	m_sUserString = pszDefault;

	int nResponse = CDialog::DoModal();
	if( nResponse == IDOK ) {
		sReturn = m_sUserString;
		return TRUE;
	}

	return FALSE;
}
