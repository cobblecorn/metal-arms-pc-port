//////////////////////////////////////////////////////////////////////////////////////
// FontomaticDlg.h
//
// Author: Albert Yale
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
// 09/17/01 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if !defined(AFX_FONTOMATICDLG_H__2DB80476_49CF_11D3_973D_005004427DE5__INCLUDED_)
#define AFX_FONTOMATICDLG_H__2DB80476_49CF_11D3_973D_005004427DE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/////////////////////////////////////////////////////////////////////////////
// CFontomaticDlg dialog

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fviewport.h"
#include "fworld.h"
#include "floop.h"
#include "fresload.h"
#include "dx/fdx8vid.h"
#include "ftext.h"
#include "fdraw.h"
#include "frenderer.h"
#include "fperf.h"
#include "ftimer.h"
#include "fversion.h"
#include "ffile.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define VARS_TO_CONTROLS	FALSE
#define CONTROLS_TO_VARS	TRUE

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class CFontomaticDlg;

typedef struct
{
	CFontomaticDlg *pDlg;
	FVidWin_t VidWin;

} _GameloopParm_t;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class CFontomaticDlg : public CDialog
{
// Construction
public:
	BOOL m_bUseLoadedValues;
	volatile BOOL m_bExiting;
	COLORREF m_Color;

	f32 m_fOriginalWordSpacing, m_fOriginalLetterSpacing, m_fOriginalItalicSlant;

	CString m_sUserName;
	CString m_sComputerName;
	CString m_sFontName;
	CString m_sMasterfileNameAndPath; //this could either be single char or UNICODE
	char m_szMasterfileNameAndPath[ MAX_PATH ]; //use this as the interface to fang for the master filename
	CFontomaticDlg(CWnd* pParent = NULL);	// standard constructor
	void ReadFnt( BOOL bImporting );
	void GenerateFont( BOOL bScriptOnly );
	void PromptUserToSaveFont( BOOL bSaveToDisk );

// Dialog Data
	//{{AFX_DATA(CFontomaticDlg)
	enum { IDD = IDD_FONTOMATIC_DIALOG };
	CEdit	m_oOutput;
	CEdit	m_oOutputDir;
	CEdit	m_oEditMasterFile;
	CButton	m_oRunScript;
	CButton	m_oRun;
	CButton	m_oOutputScriptFileBrowse;
	CButton	m_oLoad;
	CButton	m_oInputFileBrowse;
	CEdit	m_oInputFile;
	CButton	m_oInputCharsBrowse;
	CEdit	m_oEditSpacingWord;
	CEdit	m_oEditSpacingLine;
	CEdit	m_oEditSpacingLetter;
	CEdit	m_oEditNumberLines;
	CEdit	m_oEditItalicSlant;
	CButton	m_oButtonVidMode;
	CButton	m_oButtonPreview;
	CButton	m_oButtonMasterFile;
	CEdit	m_oCharString;
	CString	m_sInputFile;
	CString	m_sMasterFile;
	CString	m_EditPreview;
	int		m_oSliderItalicSlant;
	int		m_oSliderNumberLines;
	int		m_oSliderSpacingLetter;
	int		m_oSliderSpacingLine;
	int		m_oSliderSpacingWord;
	BOOL	m_oCheckDrawGuides;
	BOOL	m_oCheckFixedWidth;
	BOOL	m_oCheckItalic;
	CString	m_oEditPreview;
	BOOL	m_oOneToOne;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFontomaticDlg)
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

	// Generated message map functions
	//{{AFX_MSG(CFontomaticDlg)
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnInputFileBrowse();
	afx_msg void OnRun();
	afx_msg void OnOutputScriptFileBrowse();
	afx_msg void OnLoad();
	afx_msg void OnButtonVidmode();
	afx_msg void OnButtonMasterFile();
	afx_msg void OnButtonPreview();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnInputCharsBrowse();
	afx_msg void OnRunScript();
	afx_msg void OnCheckFixedWidth();
	afx_msg void OnChangeEditPreview();
	afx_msg void OnCheckDrawGuides();
	afx_msg void OnCheckItalic();
	afx_msg void OnCheckOneToOne();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL m_bValidVideoMode;
	_GameloopParm_t m_GameloopParm;
	CString m_oTempString;

	void SaveSettingsToDisk();
	void FreeAllocatedFontMem();

}; // CFontomaticDlg

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FONTOMATICDLG_H__2DB80476_49CF_11D3_973D_005004427DE5__INCLUDED_)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
