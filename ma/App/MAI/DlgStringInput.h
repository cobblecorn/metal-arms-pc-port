#if !defined(AFX_DLGSTRINGINPUT_H__05AB935F_C2E5_4EFB_9087_5AE06B5735D8__INCLUDED_)
#define AFX_DLGSTRINGINPUT_H__05AB935F_C2E5_4EFB_9087_5AE06B5735D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgStringInput.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DlgStringInput dialog

class DlgStringInput : public CDialog
{
// Construction
public:
	DlgStringInput::DlgStringInput(const char* pszInitString, const char* pszDlgTitle, CWnd* pParent =NULL);
private:
	DlgStringInput(CWnd* pParent = NULL);   // standard constructor
public:
// Dialog Data
	//{{AFX_DATA(DlgStringInput)
	enum { IDD = IDD_STRINGINPUT };
	CString	m_String;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgStringInput)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgStringInput)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	CString m_TitleString;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSTRINGINPUT_H__05AB935F_C2E5_4EFB_9087_5AE06B5735D8__INCLUDED_)
