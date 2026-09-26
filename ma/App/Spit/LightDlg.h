#if !defined(AFX_LIGHTDLG_H__8A06D9F9_7899_4BBD_BD02_98B1311ED785__INCLUDED_)
#define AFX_LIGHTDLG_H__8A06D9F9_7899_4BBD_BD02_98B1311ED785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LightDlg.h : header file
//

#include "fang.h"
#include "wcSliderButton.h"
#include "fcolor.h"

typedef struct {
	// starting light settings
	CFColorRGB StartLightRGB;
	f32 fStartLightIntensity;
	f32 fStartLightRadiusMultiplier;
	CFColorRGB EndLightRGB;
	f32 fEndLightIntensity;
	f32 fEndLightRadiusMultiplier;
	f32 fSndUnitVolume;
	f32 fSndPitchMultiplier;
	f32 fMaxLightRadius;
} CLightDlg_Data_t;

/////////////////////////////////////////////////////////////////////////////
// CLightDlg dialog

class CLightDlg : public CDialog
{
	DECLARE_DYNCREATE(CLightDlg)

// Construction
public:
	CLightDlg(CWnd* pParent = NULL);   // standard constructor

	void SetData( CLightDlg_Data_t *pData );

// Dialog Data
	//{{AFX_DATA(CLightDlg)
	enum { IDD = IDD_LIGHT };
	CButton	m_ctrlPickStartColor;
	wcSliderButton	m_ctrlStartRed;
	CAMSNumericEdit	m_ctrlStartRadiusMultiplier;
	wcSliderButton	m_ctrlStartIntensity;
	wcSliderButton	m_ctrlStartGreen;
	wcSliderButton	m_ctrlStartBlue;
	CButton	m_ctrlPickEndColor;
	wcSliderButton	m_ctrlEndRed;
	CAMSNumericEdit	m_ctrlEndRadiusMultiplier;
	wcSliderButton	m_ctrlEndIntensity;
	wcSliderButton	m_ctrlEndGreen;
	wcSliderButton	m_ctrlEndBlue;
	CAMSNumericEdit m_ctrlMaxRadius;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLightDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CLightDlg_Data_t m_GameData;

	// Generated message map functions
	//{{AFX_MSG(CLightDlg)
	afx_msg void OnButtonPickStartColor();
	afx_msg void OnKillfocusEditStartBlue();
	afx_msg void OnKillfocusEditStartGreen();
	afx_msg void OnKillfocusEditStartIntensity();
	afx_msg void OnKillfocusEditStartRadiusPercent();
	afx_msg void OnKillfocusEditStartRed();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	virtual BOOL OnInitDialog();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnChangeEditStartRed();
	afx_msg void OnChangeEditStartIntensity();
	afx_msg void OnChangeEditStartGreen();
	afx_msg void OnChangeEditStartBlue();
	afx_msg void OnButtonPickEndColor();
	afx_msg void OnKillfocusEditEndBlue();
	afx_msg void OnKillfocusEditEndGreen();
	afx_msg void OnKillfocusEditEndIntensity();
	afx_msg void OnKillfocusEditEndRadiusPercent();
	afx_msg void OnKillfocusEditEndRed();
	afx_msg void OnChangeEditEndRed();
	afx_msg void OnChangeEditEndIntensity();
	afx_msg void OnChangeEditEndGreen();
	afx_msg void OnChangeEditEndBlue();
	afx_msg void OnKillfocusEditMaxRadius();
	//}}AFX_MSG
	afx_msg LONG OnSliderClose(UINT lParam, LONG wParam);
	DECLARE_MESSAGE_MAP()
public:
	float m_fSoundVolume;
	float m_fPitchMultiplier;
	afx_msg void OnEnKillfocusEditSoundVol();
	afx_msg void OnEnKillfocusEditSoundPitch();
	afx_msg void OnBnClickedMakeIntensityEqual();
	afx_msg void OnBnClickedMakeRadiusMultipierEqual();
	afx_msg void OnBnClickedMakeLightColorsEqual();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHTDLG_H__8A06D9F9_7899_4BBD_BD02_98B1311ED785__INCLUDED_)
