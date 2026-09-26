#if !defined(AFX_PICKSFXDLG_H__784E927E_D201_481A_9F32_33F1F58FFC1E__INCLUDED_)
#define AFX_PICKSFXDLG_H__784E927E_D201_481A_9F32_33F1F58FFC1E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PickSfxDlg.h : header file
//

#include "fang.h"
#include "fsndfx.h"

/////////////////////////////////////////////////////////////////////////////
// CPickSfxDlg dialog

class CPickSfxDlg : public CDialog
{
// Construction
public:
	CPickSfxDlg(CWnd* pParent = NULL);   // standard constructor

	// fill in before calling do modal, will be filled in if user hits ok
	CString m_sUserSelection;
	FSndFx_BankHandle_t m_hSfxBank;
	
// Dialog Data
	//{{AFX_DATA(CPickSfxDlg)
	enum { IDD = IDD_PICK_SFX_DIALOG };
	CComboBox	m_ctrlSfxCombo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPickSfxDlg)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPickSfxDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PICKSFXDLG_H__784E927E_D201_481A_9F32_33F1F58FFC1E__INCLUDED_)
