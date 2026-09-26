#if !defined(AFX_FORCESDLG_H__A5F77621_E8F0_45D9_B1A1_67EE3C1D1955__INCLUDED_)
#define AFX_FORCESDLG_H__A5F77621_E8F0_45D9_B1A1_67EE3C1D1955__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ForcesDlg.h : header file
//

#include "fang.h"
#include "wcSliderButton.h"
#include "fmath.h"

typedef struct {
	f32 fDragMultiplierPerSec;
	f32 fGravityPerSec;
	CFVec3 MaxBubbleXYZ;
	f32 fBubbleUpdateChance;
	f32 fBubbleDragPerSec;
} CForcesDlg_Data_t;

/////////////////////////////////////////////////////////////////////////////
// CForcesDlg dialog

class CForcesDlg : public CDialog
{
	DECLARE_DYNCREATE(CForcesDlg)

// Construction
public:
	CForcesDlg(CWnd* pParent = NULL);   // standard constructor

	void SetData( CForcesDlg_Data_t *pData );

// Dialog Data
	//{{AFX_DATA(CForcesDlg)
	enum { IDD = IDD_FORCES };
	wcSliderButton	m_ctrlBubbleUpdateChance;
	CAMSNumericEdit	m_ctrlBubbleZ;
	CAMSNumericEdit	m_ctrlBubbleY;
	CAMSNumericEdit	m_ctrlBubbleX;
	CAMSNumericEdit	m_ctrlBubbleDrag;
	CAMSNumericEdit	m_ctrlGravity;
	CAMSNumericEdit	m_ctrlDrag;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CForcesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	CForcesDlg_Data_t m_GameData;

	// Generated message map functions
	//{{AFX_MSG(CForcesDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	afx_msg void OnResetDrag();
	afx_msg void OnResetGravity();
	afx_msg void OnKillfocusEditDrag();
	afx_msg void OnKillfocusEditGravity();
	afx_msg void OnEarthGravity();
	afx_msg void OnKillfocusEditBubbleDrag();
	afx_msg void OnKillfocusEditBubbleX();
	afx_msg void OnKillfocusEditBubbleY();
	afx_msg void OnKillfocusEditBubbleZ();
	afx_msg void OnKillfocusEditBubbleUpdateChance();
	afx_msg void OnChangeEditBubbleUpdateChance();
	//}}AFX_MSG
	afx_msg LONG OnSliderClose(UINT lParam, LONG wParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FORCESDLG_H__A5F77621_E8F0_45D9_B1A1_67EE3C1D1955__INCLUDED_)
