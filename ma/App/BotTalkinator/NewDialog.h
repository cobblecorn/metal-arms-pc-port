#if !defined(AFX_NEWDIALOG_H__7B442306_12C3_482D_ACF1_F39DB59FD70B__INCLUDED_)
#define AFX_NEWDIALOG_H__7B442306_12C3_482D_ACF1_F39DB59FD70B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewDialog.h : header file
//

#include "Dialogue.h"

/////////////////////////////////////////////////////////////////////////////
// CNewDialog dialog

class CNewDialog : public CDialog
{
// Construction
public:
	CNewDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewDialog)
	enum { IDD = IDD_DIALOG1 };
	CString	m_strTitle;
	CString	m_strBotName;
	//}}AFX_DATA

	CScrollBar m_oScrollBar;

	BOOL DoGetNewDialog(CDialogueInstN *pNewDI);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	CDialogueInstN *poTempDI;

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewDialog)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWDIALOG_H__7B442306_12C3_482D_ACF1_F39DB59FD70B__INCLUDED_)
