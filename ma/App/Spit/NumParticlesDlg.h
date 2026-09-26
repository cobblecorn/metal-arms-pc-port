#if !defined(AFX_NUMPARTICLESDLG_H__B03D0ECE_B5BC_46D2_816D_9DF47DE35C90__INCLUDED_)
#define AFX_NUMPARTICLESDLG_H__B03D0ECE_B5BC_46D2_816D_9DF47DE35C90__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NumParticlesDlg.h : header file
//

#include "fang.h"
#include "wcSliderButton.h"
#include "fmath.h"

typedef struct {
	CFVec2 NumPerBurst;
	CFVec2 SecsBetweenBursts;
	f32 fSecsToEmit;
	CFVec2 EmitPause;
} CNumParticlesDlg_Data_t;

/////////////////////////////////////////////////////////////////////////////
// CNumParticlesDlg dialog

class CNumParticlesDlg : public CDialog
{
	DECLARE_DYNCREATE(CNumParticlesDlg)

// Construction
public:
	CNumParticlesDlg(CWnd* pParent = NULL);   // standard constructor

	void SetData( CNumParticlesDlg_Data_t *pData );

// Dialog Data
	//{{AFX_DATA(CNumParticlesDlg)
	enum { IDD = IDD_NUM_PARTICLES };
	CAMSNumericEdit	m_ctrlMaxBurstsPerSec;
	CAMSNumericEdit	m_ctrlSecsToEmit;
	wcSliderButton	m_ctrlMaxParticles;
	wcSliderButton	m_ctrlMinParticles;
	CAMSNumericEdit	m_ctrlMinBurstsPerSec;
	CAMSNumericEdit m_ctrlMinDelaySecs;
	CAMSNumericEdit m_ctrlMaxDelaySecs;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNumParticlesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	CNumParticlesDlg_Data_t m_GameData;

	// Generated message map functions
	//{{AFX_MSG(CNumParticlesDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	afx_msg void OnKillfocusEditSecsToEmit();
	afx_msg void OnKillfocusEditMinParticles();
	afx_msg void OnKillfocusEditMaxParticles();
	afx_msg void OnKillfocusEditMinBurstsPerSec();
	afx_msg void OnChangeEditMaxParticles();
	afx_msg void OnChangeEditMinParticles();
	afx_msg void OnKillfocusEditMaxBurstsPerSec();
	afx_msg void OnKillfocusEditMinDelaySec();
	afx_msg void OnKillfocusEditMaxDelaySec();
	//}}AFX_MSG
	afx_msg LONG OnSliderClose(UINT lParam, LONG wParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NUMPARTICLESDLG_H__B03D0ECE_B5BC_46D2_816D_9DF47DE35C90__INCLUDED_)
