#if !defined(AFX_COLORDLG_H__684B7851_8829_46F8_824D_FB64B717AAFA__INCLUDED_)
#define AFX_COLORDLG_H__684B7851_8829_46F8_824D_FB64B717AAFA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorDlg.h : header file
//

#include "fang.h"
#include "wcSliderButton.h"
#include "fmath.h"

typedef struct {
	CFVec2 InitRed;
	CFVec2 InitGreen;
	CFVec2 InitBlue;	
	CFVec2 FinalRed;
	CFVec2 FinalGreen;
	CFVec2 FinalBlue;
	CFVec2 InitAlpha;	
	CFVec2 FinalAlpha;
} CColorDlg_Data_t;

/////////////////////////////////////////////////////////////////////////////
// CColorDlg dialog

class CColorDlg : public CDialog
{
	DECLARE_DYNCREATE(CColorDlg)

// Construction
public:
	CColorDlg(CWnd* pParent = NULL);   // standard constructor

	void SetData( CColorDlg_Data_t *pData );

// Dialog Data
	//{{AFX_DATA(CColorDlg)
	enum { IDD = IDD_COLOR };
	CButton	m_ctrlPickMinInitColor;
	CButton	m_ctrlPickMinFinalColor;
	CButton	m_ctrlPickMaxInitColor;
	CButton	m_ctrlPickMaxFinalColor;
	wcSliderButton	m_ctrlInitMinRed;
	wcSliderButton	m_ctrlInitMinGreen;
	wcSliderButton	m_ctrlInitMinBlue;
	wcSliderButton	m_ctrlInitMinAlpha;
	wcSliderButton	m_ctrlInitMaxRed;
	wcSliderButton	m_ctrlInitMaxGreen;
	wcSliderButton	m_ctrlInitMaxBlue;
	wcSliderButton	m_ctrlInitMaxAlpha;
	wcSliderButton	m_ctrlFinalMinRed;
	wcSliderButton	m_ctrlFinalMinGreen;
	wcSliderButton	m_ctrlFinalMinBlue;
	wcSliderButton	m_ctrlFinalMinAlpha;
	wcSliderButton	m_ctrlFinalMaxRed;
	wcSliderButton	m_ctrlFinalMaxGreen;
	wcSliderButton	m_ctrlFinalMaxBlue;
	wcSliderButton	m_ctrlFinalMaxAlpha;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CColorDlg_Data_t m_GameData;

	// Generated message map functions
	//{{AFX_MSG(CColorDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	afx_msg void OnButtonPickMaxFinalColor();
	afx_msg void OnButtonPickMaxInitColor();
	afx_msg void OnButtonPickMinFinalColor();
	afx_msg void OnButtonPickMinInitColor();
	afx_msg void OnKillfocusEditFinalMaxAlpha();
	afx_msg void OnKillfocusEditFinalMinAlpha();
	afx_msg void OnKillfocusEditFinalMaxRGB();
	afx_msg void OnKillfocusEditFinalMinRGB();
	afx_msg void OnKillfocusEditInitMaxAlpha();
	afx_msg void OnKillfocusEditInitMinAlpha();
	afx_msg void OnKillfocusEditInitMaxRGB();
	afx_msg void OnKillfocusEditInitMinRGB();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnChangeEditInitMinRed();
	afx_msg void OnChangeEditInitMinGreen();
	afx_msg void OnChangeEditInitMinBlue();
	afx_msg void OnChangeEditInitMinAlpha();
	afx_msg void OnChangeEditInitMaxRed();
	afx_msg void OnChangeEditInitMaxGreen();
	afx_msg void OnChangeEditInitMaxBlue();
	afx_msg void OnChangeEditInitMaxAlpha();
	afx_msg void OnChangeEditFinalMinRed();
	afx_msg void OnChangeEditFinalMinGreen();
	afx_msg void OnChangeEditFinalMinBlue();
	afx_msg void OnChangeEditFinalMinAlpha();
	afx_msg void OnChangeEditFinalMaxRed();
	afx_msg void OnChangeEditFinalMaxGreen();
	afx_msg void OnChangeEditFinalMaxBlue();
	afx_msg void OnChangeEditFinalMaxAlpha();
	//}}AFX_MSG
	afx_msg LONG OnSliderClose(UINT lParam, LONG wParam);
	DECLARE_MESSAGE_MAP()

private:
	FINLINE void ColorPicker( wcSliderButton *pRIntEdit, wcSliderButton *pGIntEdit, wcSliderButton *pBIntEdit, CButton *pButton );
public:
	afx_msg void OnBnClickedMakeInitColorsEqual();
	afx_msg void OnBnClickedMakeFinalColorsEqual();
	afx_msg void OnBnClickedMakeInitAlphaEqual();
	afx_msg void OnBnClickedMakeFinalAlphaEqual();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORDLG_H__684B7851_8829_46F8_824D_FB64B717AAFA__INCLUDED_)
