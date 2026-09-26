#if !defined(AFX_SCALEDLG_H__9C84D024_7CF5_45FB_98A4_429C90A1B79F__INCLUDED_)
#define AFX_SCALEDLG_H__9C84D024_7CF5_45FB_98A4_429C90A1B79F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScaleDlg.h : header file
//

#include "fang.h"
#include "fmath.h"

typedef struct {
	CFVec2 InitScale;
	CFVec2 FinalScale;	
} CScaleDlg_Data_t;

/////////////////////////////////////////////////////////////////////////////
// CScaleDlg dialog

class CScaleDlg : public CDialog
{
	DECLARE_DYNCREATE(CScaleDlg)

// Construction
public:
	CScaleDlg(CWnd* pParent = NULL);   // standard constructor

	void SetData( CScaleDlg_Data_t *pData );

// Dialog Data
	//{{AFX_DATA(CScaleDlg)
	enum { IDD = IDD_SCALE };
	CAMSNumericEdit	m_ctrlMinInitScale;
	CAMSNumericEdit	m_ctrlMinFinalScale;
	CAMSNumericEdit	m_ctrlMaxInitScale;
	CAMSNumericEdit	m_ctrlMaxFinalScale;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScaleDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CScaleDlg_Data_t m_GameData;

	// Generated message map functions
	//{{AFX_MSG(CScaleDlg)
	afx_msg void OnKillfocusEditMinInitScale();
	afx_msg void OnKillfocusEditMinFinalScale();
	afx_msg void OnKillfocusEditMaxInitScale();
	afx_msg void OnKillfocusEditMaxFinalScale();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCALEDLG_H__9C84D024_7CF5_45FB_98A4_429C90A1B79F__INCLUDED_)
