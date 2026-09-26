#if !defined(AFX_SPRITEDLG_H__3BC4A1BB_2E16_4CDF_B16B_7E665BF29E4B__INCLUDED_)
#define AFX_SPRITEDLG_H__3BC4A1BB_2E16_4CDF_B16B_7E665BF29E4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpriteDlg.h : header file
//

#include "fang.h"
#include "wcSliderButton.h"

typedef struct {
	f32 fNumSprites;
	f32 fMaxVel2;
	f32 fAlphaStepPerSprite;
	f32 fSizeStepPerSprite;
	f32 fPosStepPerSprite;	
} CSpriteDlg_Data_t;

/////////////////////////////////////////////////////////////////////////////
// CSpriteDlg dialog

class CSpriteDlg : public CDialog
{
	DECLARE_DYNCREATE(CSpriteDlg)

// Construction
public:
	CSpriteDlg(CWnd* pParent = NULL);   // standard constructor
	
	void SetData( CSpriteDlg_Data_t *pData );

// Dialog Data
	//{{AFX_DATA(CSpriteDlg)
	enum { IDD = IDD_SPRITES };
	CAMSNumericEdit	m_ctrlSizePercent;
	CAMSNumericEdit	m_ctrlPosStep;
	wcSliderButton	m_ctrlNumSprites;
	CAMSNumericEdit	m_ctrlSpriteVel;
	CAMSNumericEdit	m_ctrlAlphaPercent;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpriteDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CSpriteDlg_Data_t m_GameData;

	// Generated message map functions
	//{{AFX_MSG(CSpriteDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	afx_msg void OnKillfocusEditAlphaPercentPerSprite();
	afx_msg void OnKillfocusEditMaxSpriteVel();
	afx_msg void OnKillfocusEditNumSprites();
	afx_msg void OnKillfocusEditPosPerSprite();
	afx_msg void OnKillfocusEditSizePercentPerSprite();
	afx_msg void OnChangeEditNumSprites();
	//}}AFX_MSG
	afx_msg LONG OnSliderClose(UINT lParam, LONG wParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPRITEDLG_H__3BC4A1BB_2E16_4CDF_B16B_7E665BF29E4B__INCLUDED_)
