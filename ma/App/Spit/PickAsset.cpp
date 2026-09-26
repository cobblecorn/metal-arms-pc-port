//////////////////////////////////////////////////////////////////////////////////////
// PickAsset.cpp - 
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
#include "stdafx.h"
#include "PickAsset.h"
#include "fdata.h"

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

#define VARS_TO_CONTROLS	FALSE
#define CONTROLS_TO_VARS	TRUE

/////////////////////////////////////////////////////////////////////////////
// CPickAsset dialog


CPickAsset::CPickAsset(CWnd* pParent /*=NULL*/)
	: CDialog(CPickAsset::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPickAsset)
	m_sMasterFilename = _T("");
	m_sAssetName = _T("");
	//}}AFX_DATA_INIT

	m_hAccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDD));

	m_nOptions = PICK_ASSET_NONE;	
	m_bReadOnly = FALSE;
	m_sInitialSelection.Empty();
}


void CPickAsset::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPickAsset)
	DDX_Control(pDX, IDC_FILE_LIST, m_ctrlFileList);
	DDX_Control(pDX, IDC_FILETYPE_COMBO, m_comboTypes);
	DDX_Text(pDX, IDC_MASTER_FILENAME, m_sMasterFilename);
	DDX_Text(pDX, IDC_FILENAME, m_sAssetName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPickAsset, CDialog)
	//{{AFX_MSG_MAP(CPickAsset)
	ON_CBN_SELCHANGE(IDC_FILETYPE_COMBO, OnSelchangeFiletypeCombo)
	ON_NOTIFY(NM_CLICK, IDC_FILE_LIST, OnClickFileList)
	ON_NOTIFY(NM_DBLCLK, IDC_FILE_LIST, OnDblclkFileList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_FILE_LIST, OnItemchangedFileList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPickAsset message handlers

void CPickAsset::OnSelchangeFiletypeCombo() {
	
	UpdateData( CONTROLS_TO_VARS );

	// grab the current selection's index
	int nCurIndex = m_comboTypes.GetCurSel();

	CString sExt;
	POSITION pos;
	pos = m_asFileExtensions.FindIndex( nCurIndex );
	sExt = m_asFileExtensions.GetAt( pos );

	m_ctrlFileList.SetFocus();

	// fill in the file list based on the selected extension
	FillFileList( &sExt, NULL );	
}

void CPickAsset::OnClickFileList(NMHDR* pNMHDR, LRESULT* pResult) {
	
	SelectionChanged();

	*pResult = 0;
}

void CPickAsset::OnDblclkFileList( NMHDR* pNMHDR, LRESULT* pResult ) {

	UpdateData( CONTROLS_TO_VARS );
	
	this->PostMessage( WM_COMMAND, IDOK, 0 );
	
	*pResult = 0;
}

int CPickAsset::DoModal() {
	
	if( m_sInputMasterFile.IsEmpty() ) {
		MessageBox( _T("No master file specified,\ncan't create asset browser dialog."), 
					_T("Error"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		return IDCANCEL;
	}
	if( m_nOptions == PICK_ASSET_NONE ) {
		m_nOptions = PICK_ASSET_ALL;
	}

	m_sMasterFilename = m_sInputMasterFile;
	m_sAssetName = "";

	// read the master file
	if( m_MasterFile.ReadHeaderAndEntries( m_sMasterFilename, m_bReadOnly ) != MASTER_FILE_COMPILE_ERROR_NONE ) {
		MessageBox( _T("Could not read the master file, \ncan't create asset browser dialog."), 
					_T("Error"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		return IDCANCEL;
	}

	m_nPlatformMask = m_MasterFile.GetPlatformMask();
	if( m_nPlatformMask & FVERSION_PLATFORM_FLAG_XBOX ) {
		m_nPlatformMask = FVERSION_PLATFORM_FLAG_XBOX;
	} else if( m_nPlatformMask & FVERSION_PLATFORM_FLAG_PC ) {
		m_nPlatformMask = FVERSION_PLATFORM_FLAG_PC;		
	} else {
		// unknown format
		MessageBox( _T("The selected master file\nis for an unknown platform."), 
					_T("Error"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return IDCANCEL;
	}

	return CDialog::DoModal();
}

BOOL CPickAsset::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	// extend the list control to full row select
	DWORD dwStyle = m_ctrlFileList.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;
	m_ctrlFileList.SetExtendedStyle(dwStyle);

	// setup the file types combo box
	m_comboTypes.ResetContent();
	m_asFileExtensions.RemoveAll();
	
	// find the default selection in the master file
	CString sTemp;
	FDataPrjFile_Entry_t *pEntry = m_MasterFile.GetEntryByName( m_sInitialSelection );
	if( pEntry ) {
		sTemp = pEntry->szFilename;
		sTemp.MakeLower();
	}
	u32 nTypeCount = 0;
	s32 nInitialType = -1;
	
	if( m_nOptions & PICK_ASSET_MESHES ) {
		m_comboTypes.AddString( "Mesh Files (*.ape)" );
		m_asFileExtensions.AddTail( ".ape" );
		
		if( nInitialType < 0 && !sTemp.IsEmpty() ) {
			if( sTemp.Find( ".ape" ) >= 0 ) {
				nInitialType = nTypeCount;
			}
		}
		nTypeCount++;
	}
	if( m_nOptions & PICK_ASSET_WORLDS ) {
		m_comboTypes.AddString( "World Files (*.wld)" );
		m_asFileExtensions.AddTail( ".wld" );

		if( nInitialType < 0 && !sTemp.IsEmpty() ) {
			if( sTemp.Find( ".wld" ) >= 0 ) {
				nInitialType = nTypeCount;
			}
		}
		nTypeCount++;
	}
	if( m_nOptions & PICK_ASSET_TEXTURES ) {
		m_comboTypes.AddString( "Texture Files (*.tga)" );
		m_asFileExtensions.AddTail( ".tga" );

		if( nInitialType < 0 && !sTemp.IsEmpty() ) {
			if( sTemp.Find( ".tga" ) >= 0 ) {
				nInitialType = nTypeCount;
			}
		}
		nTypeCount++;
	}
	if( m_nOptions & PICK_ASSET_ANIMATIONS ) {
		m_comboTypes.AddString( "Animation Files (*.mtx)" );
		m_asFileExtensions.AddTail( ".mtx" );

		if( nInitialType < 0 && !sTemp.IsEmpty() ) {
			if( sTemp.Find( ".mtx" ) >= 0 ) {
				nInitialType = nTypeCount;
			}
		}
		nTypeCount++;
	}
	if( m_nOptions & PICK_ASSET_SOUNDS ) {
		m_comboTypes.AddString( "Sound Files (*.wav)" );
		m_asFileExtensions.AddTail( ".wav" );

		if( nInitialType < 0 && !sTemp.IsEmpty() ) {
			if( sTemp.Find( ".wav" ) >= 0 ) {
				nInitialType = nTypeCount;
			}
		}
		nTypeCount++;
		
	}
	if( m_nOptions & PICK_ASSET_ALL ) {
		m_comboTypes.AddString( "All Files (*.*)" );
		m_asFileExtensions.AddTail( "" );// this string is a special case and needs to be zero length

		if( nInitialType < 0 && !sTemp.IsEmpty() ) {
			nInitialType = nTypeCount;
		}
		nTypeCount++;
	}

	// set the first file type as the current selection
	if( nInitialType < 0 ) {
		nInitialType = 0;
	}
	m_comboTypes.SetCurSel( nInitialType );

	UpdateData( VARS_TO_CONTROLS );

	// grab the first selection's text
	CString sExt;
	POSITION pos;
	pos = m_asFileExtensions.FindIndex( nInitialType );
	sExt = m_asFileExtensions.GetAt( pos );
	
	// fill in the file list based on the selected extension
	if( sTemp.IsEmpty() ) {
		FillFileList( &sExt, NULL );
	} else {
		FillFileList( &sExt, &sTemp );
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPickAsset::FillFileList( CString *pExtension, CString *pInitialSelection ) {
	
	UpdateData( CONTROLS_TO_VARS );

	// Delete the current contents
	m_ctrlFileList.DeleteAllItems();
	m_sAssetName.Empty();

	u32 nNumEntries = m_MasterFile.GetNumEntries();
	if( !nNumEntries ) {
		return;
	}

	if( pExtension ) {
		pExtension->MakeLower();
		if( pExtension->IsEmpty() ) {
			// an empty string is handle just like a NULL string, 
			// display all files 
			pExtension = NULL;
		}
	}
	
	// Use the LV_ITEM structure to insert the items
	LVITEM lvi;
	CString strItem;
	const FDataPrjFile_Entry_t *pEntry;
	u32 nElements = 0;
	s32 nInitialElement = -1;
	for( u32 i=0; i < nNumEntries; i++ ) {
		// grab the entry
		pEntry = m_MasterFile.GetEntryByIndex( i );
		if( pEntry ) {
			// Insert the first item
			lvi.mask =  LVIF_TEXT;
			strItem = pEntry->szFilename;
			strItem.MakeLower();
			if( pExtension ) {
				if( strItem.Find( *pExtension ) < 0 ) {
					continue;
				}
			}
			lvi.iItem = nElements++;
			lvi.iSubItem = 0;
			lvi.pszText = (LPTSTR)(LPCTSTR)strItem;
			
			m_ctrlFileList.InsertItem( &lvi );			
		}
	}
	if( nElements ) {
		if( pInitialSelection ) {
			LVFINDINFO info;
			
			info.flags = LVFI_STRING;
			info.psz = *pInitialSelection;

			nInitialElement = m_ctrlFileList.FindItem( &info );			
		}
		if( nInitialElement < 0 ) {
			nInitialElement = 0;
		}
		// select an element to highlight in the list
		m_ctrlFileList.SetItemState( nInitialElement, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );		
		m_ctrlFileList.EnsureVisible( nInitialElement, FALSE );	
	}

	UpdateData( VARS_TO_CONTROLS );	
}

void CPickAsset::OnItemchangedFileList( NMHDR* pNMHDR, LRESULT* pResult ) {
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	SelectionChanged();
	
	*pResult = 0;
}

void CPickAsset::SelectionChanged() {
	POSITION Pos;
	int nIndex;
	
	UpdateData( CONTROLS_TO_VARS );
	Pos = m_ctrlFileList.GetFirstSelectedItemPosition();
	if( Pos ) {
		nIndex = m_ctrlFileList.GetNextSelectedItem( Pos );
		m_sAssetName = m_ctrlFileList.GetItemText( nIndex, 0 );
		UpdateData( VARS_TO_CONTROLS );	
	}	
}

BOOL CPickAsset::PreTranslateMessage( MSG* pMsg ) {
	
	if( m_hAccel != NULL ) {
		if( ::TranslateAccelerator( m_hWnd, m_hAccel, pMsg ) ) {
			return TRUE;
		}
	}	
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CPickAsset::OnOK() {
	
	// don't do anything if there is no entry selected
	if( m_sAssetName.IsEmpty() ) {
		return;
	}
	
	CDialog::OnOK();
}

CString CPickAsset::GetFileName() const {
	CString sReturn = m_sAssetName;

	return sReturn;
}

CString CPickAsset::GetFileExt() const {
	CString sReturn = m_sAssetName;

	int nIndex = sReturn.ReverseFind( '.' );
	if( nIndex >= 0 ) {
		sReturn.Delete( 0, nIndex+1 );
	} else {
		sReturn.Empty();
	}
	return sReturn;
}

CString CPickAsset::GetFileTitle() const {
	CString sReturn = m_sAssetName;

	int nIndex = sReturn.ReverseFind( '.' );
	if( nIndex >= 0 ) {
		sReturn.Delete( nIndex, sReturn.GetLength() - nIndex );
	}
	return sReturn;
}


