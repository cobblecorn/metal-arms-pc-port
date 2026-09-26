#if !defined(AFX_EDGEPROPSDLG_H__70E725FF_98FC_48F2_9797_F8034F366862__INCLUDED_)
#define AFX_EDGEPROPSDLG_H__70E725FF_98FC_48F2_9797_F8034F366862__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EdgePropsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEdgePropsDlg dialog
class GraphEdge;
class CEdgePropsDlg : public CDialog
{
// Construction
public:
	CEdgePropsDlg(int nVertId, GraphEdge* pEdge, CWnd* pParent = NULL);   // standard constructor
	BOOL NeedToRecalcVol(void);

// Dialog Data
	//{{AFX_DATA(CEdgePropsDlg)
	enum { IDD = IDD_EDGEPROPS };
	BOOL	m_bHazardDoor;
	BOOL	m_bHazardLift;
	int		m_nHazardId;
	float	m_fCustomHeight;
	float	m_fCustomWidth;
	BOOL	m_bCustom;
	BOOL	m_bDownOnly;
	BOOL	m_bUpOnly;
	BOOL	m_bHazardBridge;
	BOOL	m_bHazardDestructable;
	CComboBox	m_JumpTypesCombo;
	CComboBox	m_HazardCombo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEdgePropsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation
public:
	u16 m_uJumpType;
protected:

	// Generated message map functions
	//{{AFX_MSG(CEdgePropsDlg)
	afx_msg void OnCustomVol();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL m_bHadCustomVol;
	BOOL m_bCustomVol;
	GraphEdge* m_pGraphEdge;
public:
	afx_msg void OnBnClickedSelecthazardbutton();
	afx_msg void OnCbnEditchangeHazardcombo();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDGEPROPSDLG_H__70E725FF_98FC_48F2_9797_F8034F366862__INCLUDED_)
