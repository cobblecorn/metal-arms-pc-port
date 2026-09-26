// TestListCtrl1Dlg.h : header file
//

#if !defined(AFX_TESTLISTCTRL1DLG_H__2CCC2477_CF5A_4245_BD3A_1048837BC143__INCLUDED_)
#define AFX_TESTLISTCTRL1DLG_H__2CCC2477_CF5A_4245_BD3A_1048837BC143__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MyListCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CTestListCtrl1Dlg dialog

class CTestListCtrl1Dlg : public CDialog
{
// Construction
public:
	CTestListCtrl1Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CTestListCtrl1Dlg)
	enum { IDD = IDD_TESTLISTCTRL1_DIALOG };
	CMyListCtrl	m_oMyListCtrl;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestListCtrl1Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CTestListCtrl1Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTLISTCTRL1DLG_H__2CCC2477_CF5A_4245_BD3A_1048837BC143__INCLUDED_)
