//////////////////////////////////////////////////////////////////////////////////////
// QueryDetailsDlg.h - 
//
// Author: Pat MacKellar 
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 04/10/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUERYDETAILSDLG_H__F6AF226C_C987_4271_98AF_771E160856F1__INCLUDED_)
#define AFX_QUERYDETAILSDLG_H__F6AF226C_C987_4271_98AF_771E160856F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QueryDetailsDlg.h : header file
//
#include "MaiQueryDetails.h"


class CQueryDetailsDlg : public CDialog
{
// Construction
public:
	CQueryDetailsDlg(CNiList<CQueryDetails*> *pSaveList, CWnd* pParent = NULL);   // standard constructor
	~CQueryDetailsDlg(void);

	CQueryDetails* GetLastSelectedDetails(void) { return m_pCurOnDestroy;};
	CQueryDetails* GetCurSelectedDetails(void);

// Dialog Data
	//{{AFX_DATA(CQueryDetailsDlg)
	enum { IDD = IDD_QUERYCONFIG };
	CComboBox	m_SavesComboBox;
	BOOL	m_bCanJump;
	BOOL	m_b3DMover;
	BOOL	m_bSurfaceMover;
	float	m_fQueryHeight;
	float	m_fQueryWidth;
	float	m_fQuerySlopeCost;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQueryDetailsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CQueryDetailsDlg)
	afx_msg void OnBDeleteSave();
	afx_msg void OnBNewSave();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEbQueryWidth();
	afx_msg void OnChangeEbQueryHeight();
	afx_msg void OnCanJump();
	afx_msg void OnSelchangeCbsaves();
	afx_msg void OnDestroy();
	afx_msg void OnChangeEbQuerySlopeCost();
	afx_msg void On3DMover();
	afx_msg void OnSurfaceMover();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void UpdateDialog(void);

	CNiList<CQueryDetails*> *m_pSaveList;
	CQueryDetails* m_pCurOnDestroy;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUERYDETAILSDLG_H__F6AF226C_C987_4271_98AF_771E160856F1__INCLUDED_)
