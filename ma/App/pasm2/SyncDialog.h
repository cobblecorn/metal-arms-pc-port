#if !defined(AFX_SYNCDIALOG_H__00D11B49_8541_4358_8E4E_CD8E3141EF7B__INCLUDED_)
#define AFX_SYNCDIALOG_H__00D11B49_8541_4358_8E4E_CD8E3141EF7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SyncDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSyncDialog dialog
#include "FileInfo.h"

class CSyncDialog : public CDialog
{
// Construction
public:
	CSyncDialog( CWnd* pParent = NULL );   // standard constructor

	void SetXClientName( const CString &sStr )				   { m_sXClientFileDir = sStr; }
	void SetFilesToForceCompile( const CFileInfoArray &Files ) { m_FilesToForceCompile.Copy( Files ); }
// Dialog Data
	//{{AFX_DATA(CSyncDialog)
	enum { IDD = IDD_SYNC_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSyncDialog)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CWinThread *m_pWorkThread;
	CString m_sXClientFileDir;
	CFileInfoArray m_FilesToForceCompile;

	static UINT WorkerThread( LPVOID pParam );
	// Generated message map functions
	//{{AFX_MSG(CSyncDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYNCDIALOG_H__00D11B49_8541_4358_8E4E_CD8E3141EF7B__INCLUDED_)
