#if !defined(AFX_CLIFESPANDLG_H__19079ECD_ED3E_47D7_BD86_52A11C2187B3__INCLUDED_)
#define AFX_CLIFESPANDLG_H__19079ECD_ED3E_47D7_BD86_52A11C2187B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LifeSpanDlg.h : header file
//

#include "fang.h"
#include "fmath.h"

typedef struct {
	CFVec2 OOLifeSecs;// remember that this is min & max, NOT min & delta
} CLifeSpanDlg_Data_t;

/////////////////////////////////////////////////////////////////////////////
// LifeSpanDlg dialog

class CLifeSpanDlg : public CDialog
{
	DECLARE_DYNCREATE(CLifeSpanDlg)

// Construction
public:
	CLifeSpanDlg(CWnd* pParent = NULL);   // standard constructor

	void SetData( CLifeSpanDlg_Data_t *pData );

// Dialog Data
	//{{AFX_DATA(CLifeSpanDlg)
	enum { IDD = IDD_LIFESPAN };
	CAMSNumericEdit	m_ctrlMinLifeSecs;
	CAMSNumericEdit	m_ctrlMaxLifeSecs;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLifeSpanDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CLifeSpanDlg_Data_t m_GameData;

	// Generated message map functions
	//{{AFX_MSG(CLifeSpanDlg)
	afx_msg void OnKillfocusEditMinLifeSecs();
	afx_msg void OnKillfocusEditMaxLifeSecs();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIFESPANDLG_H__19079ECD_ED3E_47D7_BD86_52A11C2187B3__INCLUDED_)
