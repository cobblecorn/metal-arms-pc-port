// SpitDlg.h : header file
//

#if !defined(AFX_SPITDLG_H__3EF96F95_DAC9_45FB_BCED_99FBB06AE37C__INCLUDED_)
#define AFX_SPITDLG_H__3EF96F95_DAC9_45FB_BCED_99FBB06AE37C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "fang.h"
#include "RollupCtrl.h"
#include "fvid.h"
#include "wcSliderButton.h"

// rollup dialog files
#include "GlobalDlg.h"
#include "NumParticlesDlg.h"
#include "ForcesDlg.h"
#include "ColorDlg.h"
#include "EmissiveDlg.h"
#include "LifeSpanDlg.h"
#include "LightDlg.h"
#include "LodDlg.h"
#include "PostionDlg.h"
#include "ScaleDlg.h"
#include "SpriteDlg.h"
#include "VelocityDlg.h"
#include "fparticle.h"

class CSpitDlg;

#define SPIT_MAX_DECIMAL_PLACES		4

typedef enum {
	GAMELOOP_ERROR_NONE = 0,
	GAMELOOP_ERROR_COULD_NOT_CREATE_RENDER_WINDOW,
	GAMELOOP_ERROR_COULD_NOT_LOAD_WORLD,
	GAMELOOP_ERROR_COULD_NOT_START_LOOP,
	GAMELOOP_ERROR_COULD_NOT_GET_A_VIEWPORT,
	GAMELOOP_ERROR_COULD_NOT_ALLOCATE_PARTICLE_MEMORY,

	GAMELOOP_ERROR_COUNT
} GameloopErrors_e;

typedef struct {
	CSpitDlg *pDlg;
	FVidWin_t VidSettings;
	BOOL bGameloopRunning;
	GameloopErrors_e nErrorCode;	
} GameloopParams_t;

/////////////////////////////////////////////////////////////////////////////
// CSpitDlg dialog

class CSpitDlg : public CDialog
{
// Construction
public:
	CSpitDlg(CWnd* pParent = NULL);	// standard constructor

	///////////////////////////////////////
	// critical section vars & functions
	FParticleDef_t m_ParticleData;
	BOOL m_bDataNeedsSaving;// the update function will set to TRUE, the save function to FALSE
	BOOL m_bDataHasBeenUpdated;// the update function will set to TRUE, the gameloop to FALSE
	f32 m_fEmitY;
	CFVec3 m_EmitDir;
	f32 m_fEmitIntensity;
	void DataChanged( CSpriteDlg_Data_t *pData );
	void DataChanged( CVelocityDlg_Data_t *pData );
	void DataChanged( CScaleDlg_Data_t *pData );
	void DataChanged( CPositionDlg_Data_t *pData );
	void DataChanged( CNumParticlesDlg_Data_t *pData );
	void DataChanged( CLodDlg_Data_t *pData );
	void DataChanged( CLightDlg_Data_t *pData );
	void DataChanged( CLifeSpanDlg_Data_t *pData );
	void DataChanged( CGlobalDlg_Data_t *pData );
	void DataChanged( CForcesDlg_Data_t *pData );
	void DataChanged( CEmissiveDlg_Data_t *pData );
	void DataChanged( CColorDlg_Data_t *pData );
	///////////////////////////////////////

// Dialog Data
	//{{AFX_DATA(CSpitDlg)
	enum { IDD = IDD_SPIT_DIALOG };
	CButton	m_ctrlAmbientLight;
	wcSliderButton m_ctrlEmitIntensity;
	wcSliderButton m_ctrlEmitY;
	wcSliderButton m_ctrlEmitRot;
	CButton	m_ctrlCopyIntensity;
	CButton	m_ctrlCopyCurrent;
	CStatic	m_ctrlViewFrame;
	CButton	m_ctrlRollupArea;
	CString	m_sVersion;
	int m_nCurrentEditableIntensity;
	CString	m_sStats;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpitDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	HMENU m_hMenu;
	CRollupCtrl	m_ctrlRollup;
	CString m_sCurrentFile;
	CString m_sMasterFile;
	GameloopParams_t m_GameloopParams;
	u32 m_nSpriteRollupIndex;
	u32 m_nLightRollupIndex;
	
	// rollup ctrl dialog pages
	CGlobalDlg m_ctrlGlobal;
	CNumParticlesDlg m_ctrlNumParticles;
	CForcesDlg m_ctrlForces;
	CColorDlg m_ctrlColors;
	CEmissiveDlg m_ctrlEmissive;
	CLifeSpanDlg m_ctrlLifeSpan;
	CLightDlg m_ctrlLights;
	CLodDlg m_ctrlLod;
	CPostionDlg m_ctrlPosition;
	CScaleDlg m_ctrlScale;
	CSpriteDlg m_ctrlSprite;
	CVelocityDlg m_ctrlVelocity;

	// Generated message map functions
	//{{AFX_MSG(CSpitDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnNewFile();
	afx_msg void OnOpenFile();
	afx_msg void OnSaveFile();
	afx_msg void OnSaveAs();
	virtual void OnCancel();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	afx_msg void OnSelectMasterFile();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRadioMax();
	afx_msg void OnRadioMin();
	afx_msg void OnButtonCopySettings();
	afx_msg void OnButtonSetToDefaults();
	afx_msg void OnButtonEmitShot();
	afx_msg void OnKillfocusEditEmitIntensity();
	afx_msg void OnKillfocusEditEmitRot();
	afx_msg void OnKillfocusEditEmitY();
	afx_msg void OnCheckPause();
	afx_msg void OnCheckDrawBoundingSphere();
	afx_msg void OnCheckSlowMo();
	afx_msg void OnCheckDrawAid();
	afx_msg void OnChangeEditEmitY();
	afx_msg void OnChangeEditEmitIntensity();
	afx_msg void OnChangeEditEmitRot();
	afx_msg void OnCheckLookAtCamera();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnCheckAmbientLightOn();
	afx_msg void OnButtonPickAmbientLight();
	afx_msg void OnButtonStop();
	//}}AFX_MSG
	afx_msg LONG OnSliderClose(UINT lParam, LONG wParam);
	DECLARE_MESSAGE_MAP()

	void SaveSettingsToDisk();
	long OnStartRenderWindow( WPARAM wParam, LPARAM lParam );
	long OnLoadDefFile( WPARAM wParam, LPARAM lParam );
	long UpdateControls( WPARAM wParam, LPARAM lParam );

	void SaveData( BOOL bPrompt );
	void InitRollupDialogs();	

	// declare our friend classes
	friend class CGlobalDlg;
	friend class CNumParticlesDlg;
	friend class CForcesDlg;
	friend class CColorDlg;
	friend class CEmissiveDlg;
	friend class CLifeSpanDlg;
	friend class CLightDlg;
	friend class CLodDlg;
	friend class CPostionDlg;
	friend class CScaleDlg;
	friend class CSpriteDlg;
	friend class CVelocityDlg;
public:
	afx_msg void OnFileUpdateversion();
	afx_msg void OnBnClickedButtonCopyCurrentSettings();
	afx_msg void OnBnClickedCheckDrawLightBS();
};

FINLINE int AccuCast( f32 fNum ) {
	int n = (int)( fNum + 0.5f );

	return n;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPITDLG_H__3EF96F95_DAC9_45FB_BCED_99FBB06AE37C__INCLUDED_)
