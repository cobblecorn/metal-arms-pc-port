#if !defined(AFX_VELOCITYDLG_H__C26340F0_1DF8_49D6_8CBA_0AFF80E367B7__INCLUDED_)
#define AFX_VELOCITYDLG_H__C26340F0_1DF8_49D6_8CBA_0AFF80E367B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VelocityDlg.h : header file
//

#include "fang.h"
#include "wcSliderButton.h"
#include "fmath.h"

typedef struct {
	CFVec2 InitDirAngle;
	CFVec2 VelocityZ;
} CVelocityDlg_Data_t;

/////////////////////////////////////////////////////////////////////////////
// CVelocityDlg dialog

class CVelocityDlg : public CDialog
{
	DECLARE_DYNCREATE(CVelocityDlg)

// Construction
public:
	CVelocityDlg(CWnd* pParent = NULL);   // standard constructor

	void SetData( CVelocityDlg_Data_t *pData );

// Dialog Data
	//{{AFX_DATA(CVelocityDlg)
	enum { IDD = IDD_VELOCITY };
	CAMSNumericEdit	m_ctrlMinInitVel;
	wcSliderButton	m_ctrlMinSpreadAngle;
	wcSliderButton	m_ctrlMaxSpreadAngle;
	CAMSNumericEdit	m_ctrlMaxInitVel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVelocityDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CVelocityDlg_Data_t m_GameData;

	// Generated message map functions
	//{{AFX_MSG(CVelocityDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	afx_msg void OnKillfocusEditMaxInitVel();
	afx_msg void OnKillfocusEditMaxSpreadAngle();
	afx_msg void OnKillfocusEditMinInitVel();
	afx_msg void OnKillfocusEditMinSpreadAngle();
	afx_msg void OnChangeEditMaxSpreadAngle();
	afx_msg void OnChangeEditMinSpreadAngle();
	//}}AFX_MSG
	afx_msg LONG OnSliderClose(UINT lParam, LONG wParam);
	DECLARE_MESSAGE_MAP()

	static void GuiToGameData( CVelocityDlg_Data_t *pData, 
							   int nMinAngle, int nMaxAngle,
							   f32 fMinVel, f32 fMaxVel );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VELOCITYDLG_H__C26340F0_1DF8_49D6_8CBA_0AFF80E367B7__INCLUDED_)
