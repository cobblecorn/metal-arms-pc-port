// DlgStringInput.cpp : implementation file
//

#include "stdafx.h"
#include "k9.h"
#include "DlgStringInput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgStringInput dialog


DlgStringInput::DlgStringInput(const char* pszInitString, const char* pszDlgTitle, CWnd* pParent /*=NULL*/)
	: CDialog(DlgStringInput::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgStringInput)
	m_String = pszInitString;
	//}}AFX_DATA_INIT
	m_TitleString = pszDlgTitle;
}


void DlgStringInput::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgStringInput)
	DDX_Text(pDX, IDC_EBSTRING, m_String);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgStringInput, CDialog)
	//{{AFX_MSG_MAP(DlgStringInput)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgStringInput message handlers

BOOL DlgStringInput::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowText(m_TitleString);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
