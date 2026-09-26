// xclientDlg.h : header file
//

#if !defined(AFX_XCLIENTDLG_H__A39B04B0_1196_41ED_BFC1_C93A2596027D__INCLUDED_)
#define AFX_XCLIENTDLG_H__A39B04B0_1196_41ED_BFC1_C93A2596027D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "fang.h"
#include "SyncThread.h"
#include <vector>

/////////////////////////////////////////////////////////////////////////////
// CXclientDlg dialog

class CXclientDlg : public CDialog
{
// Construction
public:
	CXclientDlg(CWnd* pParent = NULL);	// standard constructor

	void ConfigLoad(const CString &sFile);
	void ConfigWrite(const CString &sFile);

	void SetAutoSync(BOOL bSync)								{m_bAutoSync = bSync;}
	void SetAutoLaunch(BOOL bLaunch)							{m_bAutoLaunch = bLaunch;}
	void SetAutoLaunchGame(const CString &sGame)                {m_sAutoLaunchGame = sGame;}
	void SetAutoClose(BOOL bClose)								{m_bAutoClose = bClose;}
	void SetFileList(const std::vector<CString> &List)	        {m_FileList = List;}
	std::vector<CString> &GetFileList(void)						{return m_FileList;}

	void SetProgressRange(const short &sMin, const short &sMax) {m_ProgressBar.SetRange(sMin, sMax);}
	void SetProgressPosition(const s32 &sPos)					{m_ProgressBar.SetPos(sPos);}
	void SetProgressStep(const s32 &sStep)						{m_ProgressBar.SetStep(sStep);}
	void ProgressStep(void)										{m_ProgressBar.StepIt();}

	void StatusTextSet(const CString &sStatus);
	void StatusTextClear(void);
// Dialog Data
	//{{AFX_DATA(CXclientDlg)
	enum { IDD = IDD_XCLIENT_DIALOG };
	CButton	m_QuitButton;
	CProgressCtrl	m_ProgressBar;
	CRichEditCtrl	m_TextStatus;
	CComboBox	m_GamesCombo;
	CButton	m_SyncCheck;
	CButton	m_LaunchCheck;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXclientDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	BOOL m_bAutoSync;
	BOOL m_bAutoLaunch;
	BOOL m_bAutoClose;
	CString m_sAutoLaunchGame;
	std::vector<CString> m_FileList;

	CString m_sXboxName;
	CString m_sUserName;
	CString m_sLocalMasterFile;
	CString m_sGameFile;
	BOOL m_bSyncChecked;
	BOOL m_bLaunchChecked;
	int m_sCurrentSel;

	std::vector<CString> m_GameList;

	CWinThread *m_pWorkThread;
	CSyncThread m_SyncThread;

	void _LoadGameFile(void);
	void _UpdateGameComboBox(void);
	CString _GetLaunchTitle(void);

	// Generated message map functions
	//{{AFX_MSG(CXclientDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonSettings();
	afx_msg void OnButtonGo();
	afx_msg void OnButtonQuit();
	afx_msg void OnCheckLaunchgame();
	afx_msg void OnCheckSyncmaster();
	afx_msg void OnSelchangeComboGames();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XCLIENTDLG_H__A39B04B0_1196_41ED_BFC1_C93A2596027D__INCLUDED_)
