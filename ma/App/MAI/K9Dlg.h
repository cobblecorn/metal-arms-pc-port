//////////////////////////////////////////////////////////////////////////////////////
// K9Dlg.h - 
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
// 06/05/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_K9DLG_H__677E0108_1965_11D5_AEBA_000102CDD4F3__INCLUDED_)
#define AFX_K9DLG_H__677E0108_1965_11D5_AEBA_000102CDD4F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "fvid.h"
#include "fxfm.h"
#include "fmath.h"
#include "fcolor.h"
#include "winprintf.h"

class CK9Dlg;
class CMainToolsWnd;

typedef struct {
	CK9Dlg *pDlg;
	FVidWin_t VidWin;
} _GameloopParm_t;

/////////////////////////////////////////////////////////////////////////////
// CK9Dlg dialog

class CK9Dlg : public CDialog
{
// Construction
public:
	CK9Dlg(CWnd* pParent = NULL);	// standard constructor
	~CK9Dlg();

	BOOL m_bLoad;
	BOOL m_bDrawCursor;
	CFColorRGB m_AmbientColorRGB;
	CFColorRGB m_3DBGColorRGB;
	BOOL m_bDrawPerfStats;
	BOOL m_bDrawFPS;
	BOOL m_bDrawCamInfo;
	BOOL m_bDrawLeafInfo;

	// used to communicate between fang window and our main dialog
	long OnGameloopRun( WPARAM wParam, LPARAM lParam );
	long OnGameloopTerm( WPARAM wParam, LPARAM lParam );
	long OnGameloopLoadErr( WPARAM wParam, LPARAM lParam );
	void SavePromptBeforeExit();
	
// Dialog Data
	//{{AFX_DATA(CK9Dlg)
	enum { IDD = IDD_K9_DIALOG };
	CString	m_sMasterFile;
	CString	m_sInputFilename;
	CString	m_sDisplayListDir;
	CString	m_sVersion;
	CString	m_sScreenshotDir;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CK9Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	HACCEL m_hAccel;

	long OnLaunchMsg( WPARAM wParam, LPARAM lParam );

	// Generated message map functions
	//{{AFX_MSG(CK9Dlg)	
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonLoad();
	afx_msg void OnChooseMasterFile();
	afx_msg void OnVideoMode();
	afx_msg void OnButtonRun();
	afx_msg void OnButtonSave();
	afx_msg void OnChooseDisplayListDir();
	afx_msg void On3dColor();
	afx_msg void OnDrawStats();
	afx_msg void OnDrawFpsClick();
	afx_msg void OnDrawCamStatsClick();
	afx_msg void OnChooseScreenshotDir();
	afx_msg void OnSeries();
	afx_msg void OnSingle();
	afx_msg void OnHideCursorClick();
	afx_msg void OnTgaClick();
	afx_msg void OnJpegClick();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
	afx_msg void OnSelectTool();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL m_bFangOK;
	BOOL m_bValidVideoMode;
	BOOL m_bRunning;
	_GameloopParm_t m_GameloopParm;
	CWinPrintf m_WinPrintf;
		
	BOOL GetFilenameFromFullPath( CString &sPathname, CString &sFilename );
	void SaveSettingsToDisk();
	void ReadSettingsFromDisk(cchar* pszLevelName);

};



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_K9DLG_H__677E0108_1965_11D5_AEBA_000102CDD4F3__INCLUDED_)
