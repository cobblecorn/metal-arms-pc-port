//////////////////////////////////////////////////////////////////////////////////////
// InputEmulation.h - InputEmulation header.
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
// 01/23/02 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if !defined(AFX_INPUTEMULATION_H__63FF909B_B02B_43C6_BDD3_BFF400AFD0BD__INCLUDED_)
#define AFX_INPUTEMULATION_H__63FF909B_B02B_43C6_BDD3_BFF400AFD0BD__INCLUDED_

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// InputEmulation.h : header file
//

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/////////////////////////////////////////////////////////////////////////////
// CInputEmulation dialog

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fang.h"
#include "fpadio.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class CInputEmulation : public CDialog
{
// Construction
public:
	CInputEmulation(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInputEmulation)
	enum { IDD = IDD_INPUT_EMULATION };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInputEmulation)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	u32 m_uValidSamples, m_uSample, m_uDev, m_uDelay, m_uComboboxItems, m_uSampledItems, m_uCurrentSelection, m_uInput, m_uControl, m_uAxes;
	CString m_oString, m_oCurrentSelection, m_oPreviousSelection;
	BOOL m_bResetContent, m_bSet;
	FPadio_Sample_t **m_paoSamples;
	FPadio_DeviceInfo_t m_oDeviceInfo;

	BOOL GetAttributeCheck( int nID );
	void SetAttributeCheck( int nID, BOOL bVal );
	void SetComboboxSelection( int nID, u32 uSelection );
	u32 GetComboboxSelection( int nID );
	void HideDropDownMenu( int nID );

	// Generated message map functions
	//{{AFX_MSG(CInputEmulation)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif // !defined(AFX_INPUTEMULATION_H__63FF909B_B02B_43C6_BDD3_BFF400AFD0BD__INCLUDED_)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
