//////////////////////////////////////////////////////////////////////////////////////
// PickAsset.h - 
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
// 07/20/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_PICKASSET_H__02EDAF51_797A_11D5_A741_0000C547D1B2__INCLUDED_)
#define AFX_PICKASSET_H__02EDAF51_797A_11D5_A741_0000C547D1B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PickAsset.h : header file
//

#include "fang.h"
#include "MasterFileCompile.h"
#include "resource.h"

////////////////////////////////////////////////////////////////////////
// A dialog that looks allows selection of assets from the master file.
// The behavior of this dialog is as close to CFileDialog as possible.
////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CPickAsset dialog

class CPickAsset : public CDialog
{
// Construction
public:
	CPickAsset(CWnd* pParent = NULL);   // standard constructor

	enum {
		PICK_ASSET_WORLDS		= 0x1,
		PICK_ASSET_MESHES		= 0x2,
		PICK_ASSET_TEXTURES		= 0x4,
		PICK_ASSET_ANIMATIONS	= 0x8,
		PICK_ASSET_SOUNDS		= 0x10,
		PICK_ASSET_ALL			= 0x20,// show we have an option for *.*, allowing any file to be selected

		PICK_ASSET_NONE			= 0
	};
	////////////////////////////////////////
	// must be set before calling DoModal()
	u32 m_nOptions;// must be combination of PICK_ASSET_ defines
	CString m_sInputMasterFile;
	BOOL m_bReadOnly;
	CString m_sInitialSelection;
	////////////////////////////////////////
	
	///////////////////////////////////////////////////
	// output functions, call if DoModal() returns IDOK
	// see CFileDialog() docs for more info
	///////////////////////////////////////////////////
	CString GetFileName() const;
	CString GetFileExt() const;
	CString GetFileTitle() const;

// Dialog Data
	//{{AFX_DATA(CPickAsset)
	enum { IDD = IDD_ASSET_PICKER };
	CListCtrl	m_ctrlFileList;
	CComboBox	m_comboTypes;
	CString	m_sMasterFilename;
	CString	m_sAssetName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPickAsset)
	public:
	virtual int DoModal();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HACCEL m_hAccel;

	// Generated message map functions
	//{{AFX_MSG(CPickAsset)
	afx_msg void OnSelchangeFiletypeCombo();
	afx_msg void OnClickFileList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkFileList(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnItemchangedFileList(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CMasterFileCompile m_MasterFile;
	u8 m_nPlatformMask;
	CStringList m_asFileExtensions;// a 1-1 mapping from the combo box to the file extensions, pass this to FillFileList()

	void FillFileList( CString *pExtension, CString *pInitialSelection );
	void SelectionChanged();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PICKASSET_H__02EDAF51_797A_11D5_A741_0000C547D1B2__INCLUDED_)
