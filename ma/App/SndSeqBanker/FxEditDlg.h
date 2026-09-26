#if !defined(AFX_FXEDITDLG_H__882978F2_20A1_4993_80DD_B987A14E0C2F__INCLUDED_)
#define AFX_FXEDITDLG_H__882978F2_20A1_4993_80DD_B987A14E0C2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FxEditDlg.h : header file
//

#include "fang.h"
#include "SeqBank.h"
#include "WavBank.h"

/////////////////////////////////////////////////////////////////////////////
// CFxEditDlg dialog

class CFxEditDlg : public CDialog
{
// Construction
public:
	CFxEditDlg(CWnd* pParent = NULL);   // standard constructor

	//////////////
	// input vars:
	CSeqBank *m_pSeqBank;
	CWavBank *m_pWavBank;
	SeqBank_Fx_t *m_pFx;
	
// Dialog Data
	//{{AFX_DATA(CFxEditDlg)
	enum { IDD = IDD_EDIT_FX_DIALOG };
	CSpinButtonCtrl	m_ctrlPitchSpinner;
	CSpinButtonCtrl	m_ctrlVolumeSpinner;
	CSpinButtonCtrl	m_ctrlLoopSpinner;
	CComboBox	m_ctrlWavList;
	CString	m_sFxName;
	UINT	m_nNumLoops;
	CString	m_sWavInfo;
	UINT	m_nVolume;
	int		m_nPitch;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFxEditDlg)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFxEditDlg)
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnSelchangeWavCombo();
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusVolume();
	afx_msg void OnKillfocusLoops();
	afx_msg void OnKillfocusPitch();
	afx_msg void OnKillfocusFxName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void UpdateWavInfo( WavBank_Entry_t *pEntry );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FXEDITDLG_H__882978F2_20A1_4993_80DD_B987A14E0C2F__INCLUDED_)
