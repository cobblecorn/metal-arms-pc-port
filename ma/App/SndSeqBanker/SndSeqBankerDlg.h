// SndSeqBankerDlg.h : header file
//

#if !defined(AFX_SNDSEQBANKERDLG_H__3090A458_6A68_4561_840A_217385DC2F86__INCLUDED_)
#define AFX_SNDSEQBANKERDLG_H__3090A458_6A68_4561_840A_217385DC2F86__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FileInfo.h"
#include "BankListCtrl.h"
#include "SeqBankListCtrl.h"
#include "WavBank.h"
#include "SeqBank.h"

/////////////////////////////////////////////////////////////////////////////
// CSndSeqBankerDlg dialog

class CSndSeqBankerDlg : public CDialog
{
// Construction
public:
	CSndSeqBankerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSndSeqBankerDlg)
	enum { IDD = IDD_SNDSEQBANKER_DIALOG };
	CSeqBankListCtrl m_ctrlSeqList;
	CBankListCtrl m_ctrlWavList;
	CString	m_sVersion;
	CString	m_sNumSeqBankFiles;
	CString	m_sNumWavBankFiles;
	CString	m_sCurrentSeqBank;
	CString	m_sCurrentWavBank;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSndSeqBankerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSndSeqBankerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRclickWavBankList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkWavBankList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkSeqBankList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickSeqBankList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChooseCurrentWavBank();
	afx_msg void OnChooseCurrentSeqBank();
	afx_msg void OnCreateNewBank();
	afx_msg void OnCreateSfx();
	afx_msg void OnDeleteFx();
	afx_msg void OnRemoveAllFxs();
	afx_msg void OnSaveBank();
	afx_msg void OnCheckSfxNames();
	afx_msg void EditFx();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CWavBank m_WavBank;
	CSeqBank m_SeqBank;
	BOOL m_bPromptForSave;

	void LoadWavBank();
	void UpdateWavBankListBox( BOOL bReSortList );

	void LoadSeqBank( BOOL bUseBankToolSettings );
	void UpdateSeqBankListBox( BOOL bReSortList );
	void CreateUniqueSeqName( const CString &rsWavName, CString &rsSeqName );
	void PromptToSaveBank();
	u32 PurgeFxWithBadWavRefs();	
public:
	afx_msg void OnLvnKeydownSeqBankList(NMHDR *pNMHDR, LRESULT *pResult);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SNDSEQBANKERDLG_H__3090A458_6A68_4561_840A_217385DC2F86__INCLUDED_)
