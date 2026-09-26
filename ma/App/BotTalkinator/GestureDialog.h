#if !defined(AFX_GESTUREDIALOG_H__C6B32F9E_421C_4C36_8F84_2D4382B4869C__INCLUDED_)
#define AFX_GESTUREDIALOG_H__C6B32F9E_421C_4C36_8F84_2D4382B4869C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GestureDialog.h : header file
//

#include "Dialogue.h"

/////////////////////////////////////////////////////////////////////////////
// CGestureDialog dialog

class CGestureDialog : public CDialog
{
// Construction
public:
	CGestureDialog(CWnd* pParent = NULL);   // standard constructor

	BOOL DoGetNewGesture(CCharacterInfo *poCI, CGesture **ppoG, CStringArray *pastrGestNames);

// Dialog Data
	//{{AFX_DATA(CGestureDialog)
	enum { IDD = IDD_DIALOG_NEWGESTURE };
	CComboBox	m_oComboGestID;
	//}}AFX_DATA

	CCharacterInfo *m_poCI;
	CGesture *m_poG;
	CStringArray *m_pastrGestNames;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGestureDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGestureDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GESTUREDIALOG_H__C6B32F9E_421C_4C36_8F84_2D4382B4869C__INCLUDED_)
