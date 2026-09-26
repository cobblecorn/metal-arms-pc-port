// WavCompileDlg.h : header file
//

#if !defined(AFX_WAVCOMPILEDLG_H__DDC12ED7_0397_47ED_BCA8_83CE3C9E921C__INCLUDED_)
#define AFX_WAVCOMPILEDLG_H__DDC12ED7_0397_47ED_BCA8_83CE3C9E921C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "fang.h"
#include "FileInfo.h"
#include "FileListCtrl.h"
#include "WavFile.h"
/////////////////////////////////////////////////////////////////////////////
// CWavCompileDlg dialog

class CWavCompileDlg : public CDialog
{
// Construction
public:
	CWavCompileDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CWavCompileDlg)
	enum { IDD = IDD_WAVCOMPILE_DIALOG };
	CFileListCtrl m_ctrlFileList;
	CString	m_sNumFiles;
	CString	m_sVersion;
	CString	m_sGCDir;
	CString	m_sXBDir;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWavCompileDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CWavCompileDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnDropFiles( HDROP hDropInfo );
	afx_msg void OnClearFileList();
	afx_msg void OnCompileButton();
	afx_msg void OnRclickFileList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkFileList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChooseGcDir();
	afx_msg void OnChooseXbDir();
	afx_msg void OnViewFileStats();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CFileInfoArray m_WavFiles;

	void UpdateFileListBox( BOOL bReSortList );
	BOOL ConvertWavFile_XB( const CFileInfo *pFileInfo, CWaveFile &rWavFile );
	BOOL ConvertWavFile_GC( const CFileInfo *pFileInfo, CWaveFile &rWavFile );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVCOMPILEDLG_H__DDC12ED7_0397_47ED_BCA8_83CE3C9E921C__INCLUDED_)
