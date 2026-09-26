#if !defined(AFX_LODDLG_H__EEA75DFD_DCBD_4DD6_A728_232DE03F0E8C__INCLUDED_)
#define AFX_LODDLG_H__EEA75DFD_DCBD_4DD6_A728_232DE03F0E8C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LodDlg.h : header file
//

#include "fang.h"
#include "wcSliderButton.h"

typedef struct {
	f32 fCullDist;
	f32 fStartSkipDrawDist;
	f32 fEmulationDist;
} CLodDlg_Data_t;

/////////////////////////////////////////////////////////////////////////////
// CLodDlg dialog

class CLodDlg : public CDialog
{
	DECLARE_DYNCREATE(CLodDlg)

// Construction
public:
	CLodDlg(CWnd* pParent = NULL);   // standard constructor

	void SetData( CLodDlg_Data_t *pData );

// Dialog Data
	//{{AFX_DATA(CLodDlg)
	enum { IDD = IDD_LOD };
	wcSliderButton	m_ctrlLODPercent;
	CAMSNumericEdit	m_ctrlDistance;
	CAMSNumericEdit	m_ctrlEmulationDistance;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLodDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CLodDlg_Data_t m_GameData;

	// Generated message map functions
	//{{AFX_MSG(CLodDlg)
	afx_msg void OnKillfocusEditCullDistance();
	afx_msg void OnKillfocusEditLodPercent();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditLodPercent();
	//}}AFX_MSG
	afx_msg LONG OnSliderClose(UINT lParam, LONG wParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnKillfocusEditEmulationdistance();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LODDLG_H__EEA75DFD_DCBD_4DD6_A728_232DE03F0E8C__INCLUDED_)
