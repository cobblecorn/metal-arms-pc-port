// ApeLaunchPadDlg.h : header file
//

#if !defined(AFX_APELAUNCHPADDLG_H__0AED1EFF_A321_4633_8A7C_1736CAEFDBE3__INCLUDED_)
#define AFX_APELAUNCHPADDLG_H__0AED1EFF_A321_4633_8A7C_1736CAEFDBE3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CApeLaunchPadDlg dialog

class CApeLaunchPadDlg : public CDialog
{
// Construction
public:
	CApeLaunchPadDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CApeLaunchPadDlg)
	enum { IDD = IDD_APELAUNCHPAD_DIALOG };
	CListBox	m_ctrlLogList;
	CString	m_sFontomaticLoc;
	CString	m_sJawsLoc;
	CString	m_sK9Loc;
	CString	m_sPasmLoc;
	CString	m_sVersion;
	CString	m_sCopyGoLoc;
	CString	m_sMawinLoc;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CApeLaunchPadDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	long OnMsgThat2ndAppWasRun( WPARAM wParam, LPARAM lParam );	
	long OnMsgThatLauncherWasBusy( WPARAM wParam, LPARAM lParam );
	long OnMsgThatPasmIsDone( WPARAM wParam, LPARAM lParam );	

	// Generated message map functions
	//{{AFX_MSG(CApeLaunchPadDlg)
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonPickFontomaticLoc();
	afx_msg void OnButtonPickJawsLoc();
	afx_msg void OnButtonPickK9Loc();
	afx_msg void OnButtonPickPasmLoc();
	afx_msg void OnButtonRunFontomatic();
	afx_msg void OnButtonRunJaws();
	afx_msg void OnButtonRunK9();
	afx_msg void OnButtonRunPasm();
	afx_msg void OnButtonPickCopyGoLoc();
	afx_msg void OnButtonRunCopyGo();
	afx_msg void OnButtonPickMawinLoc();
	afx_msg void OnButtonRunMawin();
	afx_msg void OnButtonInfo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL GetPathFromString( const CString &rsFullFilename, CString &rsPath ) const;
	BOOL IsFileACopyGoFile( const CString &rsFilename ) const;
	BOOL RunAFile( const CString &rsFilename );
	void RunPasm();
	BOOL SendRescanAndCompileMsgToPASM();
	BOOL SendLaunchMsgToK9();
	BOOL SendLaunchMsgToJaws();
	void AddStringToLog( cchar *pszString, BOOL bEnsureVisible=TRUE );
	BOOL ExtractWorldFilename( const CString &rsCmdParam, CString &rsFilename );
	BOOL ExtractApeFilename( const CString &rsCmdParam, CString &rsFilename );
	u32 ExtractMtxFilenames( const CString &rsCmdParam, CString &rsFilename1, CString &rsFilename2 );
	BOOL DoesFilenameContainInvalidChars( const CString &rsFilename );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APELAUNCHPADDLG_H__0AED1EFF_A321_4633_8A7C_1736CAEFDBE3__INCLUDED_)
