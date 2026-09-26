//////////////////////////////////////////////////////////////////////////////////////
// jawsDlg.h - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 07/19/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_JAWSDLG_H__1BDF7366_FE98_11D4_A008_000102CDD45D__INCLUDED_)
#define AFX_JAWSDLG_H__1BDF7366_FE98_11D4_A008_000102CDD45D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "fvid.h"
#include "Settings.h"	// Added by ClassView
#include "fxfm.h"
#include "winprintf.h"


class CJawsDlg;

typedef struct {
	CJawsDlg *pDlg;
	FVidWin_t VidWin;
} _GameloopParm_t;



/////////////////////////////////////////////////////////////////////////////
// CJawsDlg dialog

class CJawsDlg : public CDialog
{
// Construction
public:
	BOOL m_bPerfOn;
	BOOL m_bFPSOn;
	BOOL m_bDrawCursor;
	BOOL m_bOrientSet;
	void DeserializeXfm( CArchive *pArchive, CFXfm *pXfm );
	volatile BOOL m_bPauseAnim;
	volatile BOOL m_abLink[2];
	volatile BOOL m_abLoop[4];
	volatile BOOL m_bEnableAnim;
	BOOL m_bShowBound;
	BOOL m_bShowBones;
	CString m_asAnimResName[4];
	volatile BOOL m_abLoadAnim[4];
	CFVec3 m_DirLightVector;
	BOOL m_bDirOn;
	BOOL m_bAmbientOn;
	BOOL m_bBackgroundClear;
	CFColorRGB m_DirColorRGB;
	CFColorRGB m_AmbientColorRGB;
	CFColorRGB m_BackColorRGB;
	void UpdateControls( void );
	CString m_sResType;
	volatile BOOL m_bDataBeingUpdated;
	volatile BOOL m_bLoad;
	volatile BOOL m_bRunning;
	volatile BOOL m_bReady;
	volatile BOOL m_bReloadMasterFile;
	Settings_t m_Settings;
	CString m_sResName;
	CString m_sForceLOD;
	CJawsDlg(CWnd* pParent = NULL);	// standard constructor
	long OnGameloopRun( WPARAM wParam, LPARAM lParam );
	long OnGameloopTerm( WPARAM wParam, LPARAM lParam );
	long OnGameloopLoaded( WPARAM wParam, LPARAM lParam );
	long OnGameloopLoadErr( WPARAM wParam, LPARAM lParam );
	long OnGameloopLoad( WPARAM wParam, LPARAM lParam );
	long OnGameloopLoadAnim1( WPARAM wParam, LPARAM lParam );
	long OnGameloopLoadAnim2( WPARAM wParam, LPARAM lParam );
	long OnGameloopLoadAnim3( WPARAM wParam, LPARAM lParam );
	long OnGameloopLoadAnim4( WPARAM wParam, LPARAM lParam );

	afx_msg void OnNMCustomDrawLODSlider(NMHDR *pNMHDR, LRESULT *pResult);

// Dialog Data
	//{{AFX_DATA(CJawsDlg)
	enum { IDD = IDD_JAWS_DIALOG };
	CString	m_sVersion;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJawsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SerializeXfm( CArchive *pArchive, CFXfm *pXfm );
	void OnButtonAnim( u32 nTapID );
	CString m_sDefaultLoadExt;
	BOOL StoreSettingsIntoRegistry( BOOL bDiskKey );
	void GetSettingsFromRegistry( BOOL bDiskKey );
	_GameloopParm_t m_GameloopParm;
	HICON m_hIcon;
	CWinPrintf m_WinPrintf;

	long OnLaunchMsg( WPARAM wParam, LPARAM lParam );

	// Generated message map functions
	//{{AFX_MSG(CJawsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnButtonVidmode();
	afx_msg void OnButtonRun();
	afx_msg void OnButtonSettings();
	afx_msg void OnButtonLoad();
	afx_msg void OnButtonBackground();
	afx_msg void OnButtonAmbient();
	afx_msg void OnButtonDirectional();
	afx_msg void OnCheckAmbient();
	afx_msg void OnCheckBackground();
	afx_msg void OnCheckDirectional();
	afx_msg void OnButtonAnim1();
	afx_msg void OnButtonAnim2();
	afx_msg void OnButtonAnim3();
	afx_msg void OnButtonAnim4();
	afx_msg void OnButtonX1();
	afx_msg void OnCheckLink1();
	afx_msg void OnCheckLink2();
	afx_msg void OnButtonX2();
	afx_msg void OnButtonX3();
	afx_msg void OnButtonX4();
	afx_msg void OnCheckLoop1();
	afx_msg void OnCheckLoop2();
	afx_msg void OnCheckLoop3();
	afx_msg void OnCheckLoop4();
	afx_msg void OnCheckEnableAnim();
	afx_msg void OnCheckPauseAnim();
	afx_msg void OnButtonSaveconfig();
	afx_msg void OnButtonLoadconfig();
	afx_msg void OnCheckPerf();
	afx_msg void OnCheckShowBounds();
	afx_msg void OnHideCursorClick();
	afx_msg void On30fpsClick();
	afx_msg void On60fpsClick();
	afx_msg void OnRealtimeFpsClick();
	afx_msg void OnSeriesClick();
	afx_msg void OnSingleClick();
	afx_msg void OnCheckFpsClick();
	afx_msg void OnTgaClick();
	afx_msg void OnJpegClick();
	afx_msg void OnRealtimeSlow10();
	afx_msg void OnRealtimeSlow20();
	afx_msg void OnRealtimeSlow30();
	afx_msg void OnRealtimeSlow40();
	afx_msg void OnRealtimeSlow50();
	afx_msg void OnRealtimeSlow75();
	afx_msg void OnRealtimeSlow100();
	afx_msg void OnDontRenderSurface();
	afx_msg void OnDontRenderSpecular();
	afx_msg void OnCheckShowBones();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMCustomdrawFovSlider(NMHDR *pNMHDR, LRESULT *pResult);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JAWSDLG_H__1BDF7366_FE98_11D4_A008_000102CDD45D__INCLUDED_)
