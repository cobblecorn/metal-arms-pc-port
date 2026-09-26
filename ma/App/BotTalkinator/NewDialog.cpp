// NewDialog.cpp : implementation file
//

#include "stdafx.h"
#include "BotTalkinator.h"
#include "NewDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewDialog dialog


CNewDialog::CNewDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CNewDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewDialog)
	m_strTitle = _T("");
	m_strBotName = _T("");
	//}}AFX_DATA_INIT
}


void CNewDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewDialog)
	DDX_Text(pDX, IDC_EDIT_TITLE, m_strTitle);
	DDX_Text(pDX, IDC_EDIT_BOTNAME, m_strBotName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewDialog, CDialog)
	//{{AFX_MSG_MAP(CNewDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewDialog message handlers

BOOL CNewDialog::DoGetNewDialog(CDialogueInstN *pNewDI)
{
	poTempDI = pNewDI;

	int nResponse = DoModal();
	if (nResponse == IDOK)
		return(TRUE);
	else
		return(FALSE);
}

void CNewDialog::OnOK() 
{
	UpdateData( CONTROLS_TO_VARS );
	poTempDI->m_strTitle = m_strTitle;
	poTempDI->m_strName = m_strBotName;
	
	CDialog::OnOK();
}

BOOL CNewDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_strTitle = poTempDI->m_strTitle;
	m_strBotName = poTempDI->m_strName;

	UpdateData( VARS_TO_CONTROLS );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
