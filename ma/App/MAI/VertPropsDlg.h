#if !defined(AFX_VERTPROPSDLG_H__E2EBF8F7_74C5_4A88_8AE6_26F6CDCE934B__INCLUDED_)
#define AFX_VERTPROPSDLG_H__E2EBF8F7_74C5_4A88_8AE6_26F6CDCE934B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VertPropsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVertPropsDlg dialog
class GraphVert;
class CVertPropsDlg : public CDialog
{
// Construction
public:
	CVertPropsDlg(GraphVert* pVert, CWnd* pParent = NULL);   // standard constructor

	BOOL NeedToRecalcVol(void);
// Dialog Data
	//{{AFX_DATA(CVertPropsDlg)
	enum { IDD = IDD_VERTPROPS };
	int		m_nVertId;
	BOOL	m_bCustom;
	float	m_fCustomRadius;
	float	m_fCustomHeight;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVertPropsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CVertPropsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomvol();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL m_bHadCustomVol;
	BOOL m_bCustomVol;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VERTPROPSDLG_H__E2EBF8F7_74C5_4A88_8AE6_26F6CDCE934B__INCLUDED_)
