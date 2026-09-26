#if !defined(AFX_COPYPROGRESSDIALOG_H__DC0C1921_EBB6_11D0_9906_0060977F3DB0__INCLUDED_)
#define AFX_COPYPROGRESSDIALOG_H__DC0C1921_EBB6_11D0_9906_0060977F3DB0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CopyProgressDialog.h : header file
//

#include <stdio.h>
#include <time.h>

/////////////////////////////////////////////////////////////////////////////
// CCopyProgressDialog dialog

class CCopyProgressDialog : public CDialog
{
// Construction
public:
	CCopyProgressDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCopyProgressDialog)
	enum { IDD = IDD_COPY_PROGRESS };
	CStatic	m_target;
	CStatic	m_source;
	CProgressCtrl	m_progress;
	//}}AFX_DATA

	BOOL			m_cancel;

	DWORD			m_size;

	time_t			m_ctime;
	time_t			m_atime;
	time_t			m_mtime;

	const char *	m_source_name;
	const char *	m_target_name;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCopyProgressDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	afx_msg LONG CopyContinue( UINT wParam, LONG lParam );

// Implementation
protected:
	enum Constants {
		BUFFER_SIZE	= 4096,
	};

	char			m_buffer[BUFFER_SIZE];

	DWORD			m_copied;

	FILE *			m_source_handle;
	FILE *			m_target_handle;

	int				m_timer;

	int				m_last_progress;

	// Generated message map functions
	//{{AFX_MSG(CCopyProgressDialog)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COPYPROGRESSDIALOG_H__DC0C1921_EBB6_11D0_9906_0060977F3DB0__INCLUDED_)
