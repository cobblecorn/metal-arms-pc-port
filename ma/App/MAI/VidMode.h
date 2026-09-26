//////////////////////////////////////////////////////////////////////////////////////
// VidMode.h - 
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
#if !defined(AFX_VIDMODE_H__1BDF736F_FE98_11D4_A008_000102CDD45D__INCLUDED_)
#define AFX_VIDMODE_H__1BDF736F_FE98_11D4_A008_000102CDD45D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VidMode.h : header file
//

#include "fvid.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CVidMode dialog

class CVidMode : public CDialog
{
// Construction
public:
	BOOL GetVidMode( FVidWin_t *pDestVidWin );
	CVidMode(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CVidMode)
	enum { IDD = IDD_VIDMODE };
	CComboBox	m_ComboMode;
	CComboBox	m_ComboDevice;
	BOOL	m_bFullscreen;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVidMode)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	FVidMode_t m_HighlightedVidMode;
	FVidDev_t m_HighlightedVidDev;
	BOOL m_bHighlightedMode;
	FVidRenderer_t m_nDeviceType;
	FVidWin_t m_CurrentVidWin;
	BOOL m_bModeSelected;

	void SelectDefaultsInCombos( void );
	BOOL AreModesIdentical( const FVidMode_t *pMode1, const FVidMode_t *pMode2 );
	BOOL AreDevsIdentical( const FVidDev_t *pDev1, const FVidDev_t *pDev2 );
	void FillComboModeList();
	void Enumerate( void );
	BOOL StoreVidModeIntoRegistry( const FVidWin_t *pVidWin );
	BOOL GetVidModeFromRegistry( FVidWin_t *pDestVidWin );		

	// Generated message map functions
	//{{AFX_MSG(CVidMode)
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioHardware();
	afx_msg void OnRadioSoftware();
	afx_msg void OnRadioRefrast();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnCheckFullscreen();
	afx_msg void OnSelchangeComboDevice();
	afx_msg void OnSelchangeComboMode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDMODE_H__1BDF736F_FE98_11D4_A008_000102CDD45D__INCLUDED_)
