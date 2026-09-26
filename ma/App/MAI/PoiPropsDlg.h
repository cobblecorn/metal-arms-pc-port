#if !defined(AFX_POIPROPSDLG_H__0E7FDD63_7CCA_4FCE_BBB8_2ACACC9A280B__INCLUDED_)
#define AFX_POIPROPSDLG_H__0E7FDD63_7CCA_4FCE_BBB8_2ACACC9A280B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PoiPropsDlg.h : header file
//

class CGraphPoi;

/////////////////////////////////////////////////////////////////////////////
// CPoiPropsDlg dialog

class CPoiPropsDlg : public CDialog
{
// Construction
public:
	CPoiPropsDlg(CGraphPoi* pPoi, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPoiPropsDlg)
	enum { IDD = IDD_POIPROPS };
	BOOL	m_bOffensivePoi;
	BOOL	m_bNE;
	BOOL	m_bNW;
	BOOL	m_bS;
	BOOL	m_bSE;
	BOOL	m_bSW;
	BOOL	m_bW;
	BOOL	m_bE;
	BOOL	m_bN;
	BOOL	m_bCustom;
	int		m_nRange;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPoiPropsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

	CGraphPoi* m_pPoi;
protected:

	// Generated message map functions
	//{{AFX_MSG(CPoiPropsDlg)
	afx_msg void OnCustom();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POIPROPSDLG_H__0E7FDD63_7CCA_4FCE_BBB8_2ACACC9A280B__INCLUDED_)
