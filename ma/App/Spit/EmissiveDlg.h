#if !defined(AFX_EMISSIVEDLG_H__539E5CBE_60C6_46F4_A058_19A007F35CB8__INCLUDED_)
#define AFX_EMISSIVEDLG_H__539E5CBE_60C6_46F4_A058_19A007F35CB8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmissiveDlg.h : header file
//

#include "fang.h"
#include "wcSliderButton.h"
#include "fmath.h"

typedef struct {
	CFVec2 EmissiveIntensity;
} CEmissiveDlg_Data_t;

/////////////////////////////////////////////////////////////////////////////
// CEmissiveDlg dialog

class CEmissiveDlg : public CDialog
{
	DECLARE_DYNCREATE(CEmissiveDlg)

// Construction
public:
	CEmissiveDlg(CWnd* pParent = NULL);   // standard constructor

	void SetData( CEmissiveDlg_Data_t *pData );

// Dialog Data
	//{{AFX_DATA(CEmissiveDlg)
	enum { IDD = IDD_EMISSIVE };
	wcSliderButton m_ctrlMin;
	wcSliderButton m_ctrlMax;	
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmissiveDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CEmissiveDlg_Data_t m_GameData;

	// Generated message map functions
	//{{AFX_MSG(CEmissiveDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	afx_msg void OnKillfocusEditMaxEmissive();
	afx_msg void OnKillfocusEditMinEmissive();
	afx_msg void OnChangeEditMaxEmissive();
	afx_msg void OnChangeEditMinEmissive();
	//}}AFX_MSG
	afx_msg LONG OnSliderClose(UINT lParam, LONG wParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMISSIVEDLG_H__539E5CBE_60C6_46F4_A058_19A007F35CB8__INCLUDED_)
