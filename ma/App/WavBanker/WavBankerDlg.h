// WavBankerDlg.h : header file
//

#if !defined(AFX_WAVBANKERDLG_H__EB6CFD52_6E4B_4645_8A64_C1CE490B2A51__INCLUDED_)
#define AFX_WAVBANKERDLG_H__EB6CFD52_6E4B_4645_8A64_C1CE490B2A51__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FileInfo.h"
#include "FileListCtrl.h"
#include "BankListCtrl.h"
#include "WavBank.h"

/////////////////////////////////////////////////////////////////////////////
// CWavBankerDlg dialog

class CWavBankerDlg : public CDialog
{
// Construction
public:
	CWavBankerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CWavBankerDlg)
	enum { IDD = IDD_WAVBANKER_DIALOG };
	CFileListCtrl m_ctrlFileList;
	CBankListCtrl m_ctrlBankList;
	CString	m_sCurrentDir;
	CString	m_sCurrentBank;
	BOOL	m_bRecurseSubDirs;
	CString	m_sNumFilesShown;
	CString	m_sVersion;
	CString	m_sNumBankFilesShown;
	CString	m_sBankSize;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWavBankerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CWavBankerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnCancel();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnChooseCurrentDir();
	afx_msg void OnSubDirsCheck();
	afx_msg void OnChooseCurrentBank();
	afx_msg void OnRefreshButton();
	afx_msg void OnRclickFileList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkFileList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddToBank();
	afx_msg void OnRclickBankList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSaveButton();
	afx_msg void OnUpdateButton();
	afx_msg void OnDeleteButton();
	afx_msg void OnCreateNewBank();
	afx_msg void OnRemoveAllEntries();
	afx_msg void OnViewFileStats();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CFileInfoArray m_WavFiles;
	CWavBank m_WavBank;
	BOOL m_bPromptForSave;

	void ScanForWavFiles();
	void UpdateFileListBox( BOOL bReSortList );
	void PromptToSaveBank();
	void SaveBankFile();
	void LoadBankFile( BOOL bUseBankToolSettings );
	void UpdateBankListBox( BOOL bReSortList );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVBANKERDLG_H__EB6CFD52_6E4B_4645_8A64_C1CE490B2A51__INCLUDED_)
