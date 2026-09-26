#if !defined(AFX_GLOBALDLG_H__260A6388_71BD_4D3D_87B2_4AF125ABAF03__INCLUDED_)
#define AFX_GLOBALDLG_H__260A6388_71BD_4D3D_87B2_4AF125ABAF03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GlobalDlg.h : header file
//

#include "fang.h"
#include "fmath.h"
#include "ftex.h"
#include "fparticle.h"
#include "wcSliderButton.h"

typedef struct {
	u32 nFlags;
	char szTextureName[FDATA_TEXNAME_LEN+1];
	FParticleAnimFunc_t AnimAlphaInfo;
	FParticleAnimFunc_t AnimScaleInfo;
	FParticleAnimFunc_t AnimColorInfo;
	u32 nLightMotifIndex;
	f32 fSecsBetweenLightSamples;
	CFVec3 SampledLightScale;
	char szCoronaTextureName[FDATA_TEXNAME_LEN+1];
	f32 fCoronaScale;
	u32 nFlags2;
	char szSoundFxName[FDATA_TEXNAME_LEN+1];
} CGlobalDlg_Data_t;

/////////////////////////////////////////////////////////////////////////////
// CGlobalDlg dialog

class CGlobalDlg : public CDialog
{
	DECLARE_DYNCREATE(CGlobalDlg)

// Construction
public:
	CGlobalDlg(CWnd* pParent = NULL);   // standard constructor

	void SetData( CGlobalDlg_Data_t *pData );

// Dialog Data
	//{{AFX_DATA(CGlobalDlg)
	enum { IDD = IDD_GLOBAL };
	wcSliderButton	m_ctrlScaleRedSamples;
	wcSliderButton	m_ctrlScaleGreenSamples;
	wcSliderButton	m_ctrlScaleBlueSamples;
	wcSliderButton	m_ctrlSamplesPerSec;
	CAMSIntegerEdit	m_ctrlMotifIndex;
	BOOL	m_bBurstForever;
	BOOL	m_bDirectional;
	BOOL	m_bLight;
	BOOL	m_bZScale;
	BOOL	m_bIgnoreWind;
	BOOL	m_bMultiSprites;
	BOOL	m_bNoFog;
	BOOL	m_bOneShot;
	BOOL	m_bPixelSize;
	int		m_nDrawMode;
	BOOL	m_bRandFinalColor;
	BOOL	m_bRandInitColor;
	BOOL	m_bSampleLights;
	BOOL	m_bSplitOnImpact;
	BOOL	m_bZBuffer;
	CString	m_sTextureName;
	BOOL	m_bEnableTextures;
	BOOL	m_bUseBubbleForce;
	BOOL	m_bAddPosJitter;
	BOOL	m_bCloseSpriteGaps;
	BOOL	m_bPlaySound;
	BOOL	m_bPerPixelLight;
	BOOL	m_bAutoCalcLightIntensity;
	BOOL	m_bCoronaOn;
	BOOL	m_bPerspectiveScaleCorona;
	BOOL	m_bSpikeColor;
	BOOL	m_bSpikeAlpha;
	BOOL	m_bSpikeScale;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGlobalDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CGlobalDlg_Data_t m_GameData;

	// Generated message map functions
	//{{AFX_MSG(CGlobalDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	afx_msg void OnBurstForever();
	afx_msg void OnDirectionalEmission();
	afx_msg void OnEmitLight();
	afx_msg void OnGroupZScale();
	afx_msg void OnIgnoreWind();
	afx_msg void OnMultiSprites();
	afx_msg void OnNoFog();
	afx_msg void OnOneShot();
	afx_msg void OnPixelSize();
	afx_msg void OnRadioAddMode();
	afx_msg void OnRadioModulateMode();
	afx_msg void OnRandomFinalColor();
	afx_msg void OnRandomInitColor();
	afx_msg void OnSampleLights();
	afx_msg void OnSplitOnImpact();
	afx_msg void OnZBuffer();
	afx_msg void OnEnableTextures();
	afx_msg void OnChooseTexture();
	afx_msg void OnKillfocusEditMotifIndex();
	afx_msg void OnKillfocusEditSamplesPerSec();
	afx_msg void OnChangeEditSamplesPerSec();
	afx_msg void OnKillfocusEditScaleSampledBlue();
	afx_msg void OnChangeEditScaleSampledBlue();
	afx_msg void OnKillfocusEditScaleSampledGreen();
	afx_msg void OnChangeEditScaleSampledGreen();
	afx_msg void OnKillfocusEditScaleSampledRed();
	afx_msg void OnChangeEditScaleSampledRed();
	afx_msg void OnJitterPos();
	afx_msg void OnUseBubbleForce();
	afx_msg void OnCloseSpriteGaps();
	afx_msg void OnPlaySound();
	afx_msg void OnPerPixelLight();
	afx_msg void OnAutoCalcLightIntensity();
	afx_msg void OnCoronaOn();
	afx_msg void OnPerspectiveScaleCorona();
	afx_msg void OnSpikeColor();
	afx_msg void OnSpikeAlpha();
	afx_msg void OnSpikeScale();
	//}}AFX_MSG
	afx_msg LONG OnSliderClose(UINT lParam, LONG wParam);
	DECLARE_MESSAGE_MAP()

	void CGlobalDlg::UpdateFlag( u32 &rnFlags, u32 nMask, BOOL bCurrentSetting );
public:
	CString m_sSoundName;
	afx_msg void OnBnClickedChooseCoronaTexture();
	afx_msg void OnBnClickedClearCoronaTexture();
	CString m_sCoronaTexture;
	float m_fCoronaScale;
	afx_msg void OnEnKillfocusEditCoronaScale();
	afx_msg void OnEnKillfocusSoundName();
	BOOL m_bSortParticleGroup;
	afx_msg void OnBnClickedSort();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GLOBALDLG_H__260A6388_71BD_4D3D_87B2_4AF125ABAF03__INCLUDED_)
