// ma_winDlg.h : header file
//

#if !defined(AFX_MA_WINDLG_H__47B97659_697B_4588_B14F_22611086E62F__INCLUDED_)
#define AFX_MA_WINDLG_H__47B97659_697B_4588_B14F_22611086E62F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "fcolor.h"
#include "resource.h"
#include "winprintf.h"
#include "fvid.h"

/////////////////////////////////////////////////////////////////////////////
// CMa_winDlg dialog

class CMa_winDlg : public CDialog
{
// Construction
public:
	CMa_winDlg(CWnd* pParent = NULL);	// standard constructor

	CFColorRGB m_3DBGColorRGB;
	static CMa_winDlg *m_pMainDialog;

	long OnGameloopExit( WPARAM wParam, LPARAM lParam );	
	
// Dialog Data
	//{{AFX_DATA(CMa_winDlg)
	enum { IDD = IDD_MA_WIN_DIALOG };
	CString	m_sMasterFile;
	CString	m_sInputFilename;
	CString	m_sScreenshotDir;
	CString	m_sVersion;
	BOOL	m_bSkipLevelSelect;
	BOOL	m_bDebugAI;
	BOOL	m_bPlayerDeath;
	CString	m_sMemcardDir;
	BOOL	m_bShowEntityInfo;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMa_winDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMa_winDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnCancel();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnButtonLoad();
	afx_msg void OnChooseMasterFile();
	afx_msg void OnVideoMode();
	afx_msg void OnButtonRun();
	afx_msg void On3dColor();
	afx_msg void OnChooseScreenshotDir();
	afx_msg void OnSeries();
	afx_msg void OnSingle();
	afx_msg void OnViewBounds();
	afx_msg void OnDontRenderSurface();
	afx_msg void OnDontRenderSpecular();
	afx_msg void OnDontRenderTranslucencies();
	afx_msg void OnCameraPOVVisibility();
	afx_msg void OnPlayerDeath();
	afx_msg void OnDebugAI();
	afx_msg void OnBDADAI();
	afx_msg void On30fps();
	afx_msg void On60fps();
	afx_msg void OnRealtimeFps();
	afx_msg void OnInputMapper();
	afx_msg void OnJpegClick();
	afx_msg void OnTgaClick();
	afx_msg void OnXbox();
	afx_msg void OnGamecube();
	afx_msg void OnDisplayFps();
	afx_msg void OnChooseMemcardDir();
	afx_msg void OnSlot1();
	afx_msg void OnSlot2();
	afx_msg void OnDisplaySafeScreenBox();
	afx_msg void OnLegServoDefault();
	afx_msg void OnLegServoL1();
	afx_msg void OnLegServoL2();
	afx_msg void OnLegServoL3();
	afx_msg void OnArmServoDefault();
	afx_msg void OnArmServoL1();
	afx_msg void OnArmServoL2();
	afx_msg void OnArmServoL3();
	afx_msg void OnCheckShowEntityInfo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_bFangOK;
	BOOL m_bValidVideoMode;
	BOOL m_bRunning;
	CWinPrintf m_WinPrintf;
	FVidWin_t m_VidWin;
			
	BOOL GetFilenameFromFullPath( CString &sPathname, CString &sFilename );	
	void EnableControls( BOOL bEnable );	

	void UpdateLegServoCheckbox( u32 nServoLevel );
	void UpdateArmServoCheckbox( u32 nServoLevel );
	u32 GetLegServoLevel( void ) const;
	u32 GetArmServoLevel( void ) const;
public:
	afx_msg void OnBnClickedDontDrawHud();
	afx_msg void OnBnClickedDontDrawDebug();
	afx_msg void OnBnClickedCameraPovVisibility2();
	afx_msg void OnBnClickedLightShowrange();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MA_WINDLG_H__47B97659_697B_4588_B14F_22611086E62F__INCLUDED_)
