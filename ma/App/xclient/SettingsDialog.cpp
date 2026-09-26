// SettingsDialog.cpp : implementation file
//
#include "fang.h"
#include "stdafx.h"
#include "xclient.h"
#include "XboxDbg.h"
#include "SettingsDialog.h"
#include "dx/fserver.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSettingsDialog dialog


CSettingsDialog::CSettingsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSettingsDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingsDialog)
	DDX_Control(pDX, IDC_TEXT_GAMEFILE, m_EditGameFile);
	DDX_Control(pDX, IDC_TEXT_MASTERFILE, m_EditMasterFile);
	DDX_Control(pDX, IDC_TEXT_USERNAME, m_EditUserName);
	DDX_Control(pDX, IDC_TEXT_XBOX, m_EditXboxName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSettingsDialog, CDialog)
	//{{AFX_MSG_MAP(CSettingsDialog)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_REBOOT, OnReboot)
	ON_BN_CLICKED(IDC_BROWSE2, OnBrowse2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsDialog message handlers

BOOL CSettingsDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_EditXboxName.SetWindowText(m_sXboxName);
	m_EditUserName.SetWindowText(m_sUserName);
	m_EditMasterFile.SetWindowText(m_sLocalMasterFile);
	m_EditGameFile.SetWindowText(m_sGameFile);

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingsDialog::OnBrowse() 
{
	CFileDialog fileDialog(TRUE, 
		".mst", 
		NULL, 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		"Master Files (*.mst)|*.mst||", 
		this);

	fileDialog.m_ofn.lpstrTitle = "Select Master File";

	if (fileDialog.DoModal() == IDOK)
	{
		m_EditMasterFile.SetWindowText(fileDialog.GetPathName());
	}
}

// Only update information on OK, not on Cancel
void CSettingsDialog::OnOK() 
{
	m_EditXboxName.GetWindowText(m_sXboxName);
	m_EditUserName.GetWindowText(m_sUserName);
	m_EditMasterFile.GetWindowText(m_sLocalMasterFile);
	m_EditGameFile.GetWindowText(m_sGameFile);

	if (m_sXboxName.IsEmpty())
		m_sXboxName = "xbox";

	if (m_sUserName.IsEmpty())
		m_sUserName = "user";

	if (m_sUserName.GetLength() >= FSERVER_MAX_CLIENT_NAME)
	{
		m_sUserName = m_sUserName.Left(FSERVER_MAX_CLIENT_NAME - 1);
	}

	m_sUserName.Replace(' ', '_');

	CDialog::OnOK();
}

void CSettingsDialog::OnReboot() 
{
	int res = MessageBox("Are you sure you want to reboot the Xbox?", "Are you sure?", MB_YESNO | MB_ICONQUESTION);

	if (res == IDYES)
	{
		HRESULT hRes = DmSetXboxNameNoRegister(m_sXboxName);

		if (FAILED(hRes))
		{
			MessageBox("Failed to set Xbox name!", "Error", MB_ICONERROR);
			return;
		}

		hRes = DmReboot(DMBOOT_WARM);

		if (FAILED(hRes))
		{
			MessageBox("Failed to reboot Xbox!", "Error", MB_ICONERROR);
			return;
		}

		MessageBox("Xbox has been rebooted", "Reboot done");

		// Sleep for a bit while the Xbox reboots
		Sleep(1000 * 2);
	}
}

void CSettingsDialog::OnBrowse2() 
{
	CFileDialog fileDialog(TRUE, 
		".agl", 
		NULL, 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		"Ape Game List Files (*.agl)|*.agl||", 
		this);

	fileDialog.m_ofn.lpstrTitle = "Select Game File";

	if (fileDialog.DoModal() == IDOK)
	{
		m_EditGameFile.SetWindowText(fileDialog.GetPathName());	
	}
}
