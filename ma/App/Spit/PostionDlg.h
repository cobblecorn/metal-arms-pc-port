#if !defined(AFX_POSTIONDLG_H__5526C478_99BF_45D8_9F7D_281EAE4B413F__INCLUDED_)
#define AFX_POSTIONDLG_H__5526C478_99BF_45D8_9F7D_281EAE4B413F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PostionDlg.h : header file
//

#include "fang.h"
#include "fmath.h"

typedef struct {
	f32 fRandomInitPosRadius;
	CFVec3 JitterOffset;
} CPositionDlg_Data_t;

/////////////////////////////////////////////////////////////////////////////
// CPostionDlg dialog

class CPostionDlg : public CDialog
{
	DECLARE_DYNCREATE(CPostionDlg)	

// Construction
public:
	CPostionDlg(CWnd* pParent = NULL);   // standard constructor

	void SetData( CPositionDlg_Data_t *pData );

// Dialog Data
	//{{AFX_DATA(CPostionDlg)
	enum { IDD = IDD_POSITION };
	CAMSNumericEdit	m_ctrlJitterZ;
	CAMSNumericEdit	m_ctrlJitterY;
	CAMSNumericEdit	m_ctrlJitterX;
	CAMSNumericEdit	m_ctrlPosRadius;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPostionDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CPositionDlg_Data_t m_GameData;

	// Generated message map functions
	//{{AFX_MSG(CPostionDlg)
	afx_msg void OnKillfocusEditInitialPosRadius();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusEditJitterX();
	afx_msg void OnKillfocusEditJitterY();
	afx_msg void OnKillfocusEditJitterZ();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POSTIONDLG_H__5526C478_99BF_45D8_9F7D_281EAE4B413F__INCLUDED_)
