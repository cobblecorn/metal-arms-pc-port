#if !defined(AFX_DEPENDENCYDIALOG_H__CDF7154F_E997_11D0_9906_0060977F3DB0__INCLUDED_)
#define AFX_DEPENDENCYDIALOG_H__CDF7154F_E997_11D0_9906_0060977F3DB0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DependencyDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDependencyDialog dialog

class CDependencyDialog : public CDialog
{
// Construction
public:
	CDependencyDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDependencyDialog)
	enum { IDD = IDD_DEPENDENCY };
	CString	m_source;
	CString	m_destination;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDependencyDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDependencyDialog)
	afx_msg void OnSourceBrowse();
	afx_msg void OnDestinationBrowse();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEPENDENCYDIALOG_H__CDF7154F_E997_11D0_9906_0060977F3DB0__INCLUDED_)
