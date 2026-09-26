//////////////////////////////////////////////////////////////////////////////////////
// pasmDlg.h - 
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
#if !defined(AFX_PASMDLG_H__9961A1C6_54F9_11D5_AEBF_000102CDD4F3__INCLUDED_)
#define AFX_PASMDLG_H__9961A1C6_54F9_11D5_AEBF_000102CDD4F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _MMI_TARGET_PS2
#include "ps2\resource.h"
#else
#include "resource.h"
#endif
#include "FileInfo.h"
#include "MyListCtrl.h"
#include "MyItemInfo.h"
#include "MasterFileCompile.h"
#include "ToolTipButton.h"
#include "FileLock.h"

typedef enum 
{
	PASM_TARGET_NONE = 0,
	PASM_TARGET_XB,
	PASM_TARGET_PC,
    PASM_TARGET_GC,
#ifdef _MMI_TARGET_PS2                  //NS
    PASM_TARGET_PS2
#endif
} TargetPlatform_e;


/////////////////////////////////////////////////////////////////////////////
// CPasmDlg dialog

class CPasmDlg : public CDialog
{
// Construction
public:
	CPasmDlg(CWnd* pParent = NULL);	// standard constructor

	CStatusBarCtrl	m_StatusBar;
	UINT m_nLibFileCount;
	UINT m_nLocalFileCount;
	UINT m_nMasterFileCount;

	CFileInfoArray m_LocalFiles;
	CMapStringToPtr m_MapLocalFiles;// maps filenames (mike.txt) to pFileInfo pointers in m_LocalFiles

	u32 m_nLightingQuality;
	f32 m_fMaxLMapSize;
	f32 m_fSubSample;
//	BOOL m_bFilterLightMaps;

	static CString *m_pStaticLocalDir;
	static CString m_csPASMTempDirectory;

	CString	m_sLibDir;

	BOOL ScanLocalDrive();
	BOOL ScanLibDrive();

// Dialog Data
	//{{AFX_DATA(CPasmDlg)
	enum { IDD = IDD_PASM_DIALOG };
	CMyListCtrl	m_ctrlFileList;
	int		m_nLeafSize;
	BOOL	m_bAutoLeafSize;
	CToolTipButton m_CompileButton;
	CToolTipButton m_SyncDownButton;
	CToolTipButton m_SyncUpButton;
	CToolTipButton m_SyncDownAndCompileButton;
	CToolTipButton m_NewerThanLocButton;
	CToolTipButton m_NewerThanLibButton;
	CToolTipButton m_NewerThanMasterButton;
	CToolTipButton m_RescanButton;
	CToolTipButton m_ScanAndCompileButton;
	BOOL	m_bDownsizeTextures;
	BOOL	m_bDisableStripping;
	BOOL	m_bDetailedLightingInfo;
	BOOL	m_bDontUseVIS;
	BOOL	m_bReusePerfectVISMatchOnly;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPasmDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	HACCEL m_hAccel;
	HMENU m_hMenu;

	long OnCheckMasterFileVersion( WPARAM wParam, LPARAM lParam );	
	long OnExternalAppRequestingRescanAndCompile( WPARAM wParam, LPARAM lParam );

	// Generated message map functions
	//{{AFX_MSG(CPasmDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnCancel();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnKillFocusLeafSizeEdit();
	afx_msg void OnPressConfigButton();
	afx_msg void OnXBoxFormat();
	afx_msg void OnPCFormat();
	afx_msg void OnGCFormat();
#ifdef _MMI_TARGET_PS2
    afx_msg void OnPS2Format();
#endif
	afx_msg void OnOutputDebugPics();
	afx_msg void OnShowDifferences();
	afx_msg void OnHideNoncompilableFiles();
	afx_msg void OnHoldCompileStats();
	afx_msg void OnDisableLibrarySupport();
	afx_msg void OnSelectNewerThanMaster();
	afx_msg void OnSelectNewerThanLocal();
	afx_msg void OnSelectNewerThanLib();
	afx_msg void OnCompile();
	afx_msg void OnForcedCompile();
	afx_msg void OnClearErrorLog();
	afx_msg void OnRebuildMasterFile();
	afx_msg void OnRescan();
	afx_msg void OnDelete();
	afx_msg void OnOptimizeMasterfile();
	afx_msg void OnDblclkFileList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickFileList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedFileList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnScanAndCompile();
	afx_msg void OnRclickFileList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSyncDownClick();
	afx_msg void OnSyncUpClick();
	afx_msg void OnSyncDownAndCompile();
	afx_msg void OnMirrorLibrary();
	afx_msg void OnClickDetailedFileStats();
	afx_msg void OnClickOpenErrorLog();
	afx_msg void OnDumpFilenames();
	afx_msg void OnToolsCreateZipFile();
	afx_msg void OnCompactMasterFile();
	afx_msg void OnGenerateFileTypeUsageFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL m_bFangOK;

	// main dialog versions of key settings
	BOOL m_bMustRunConfig;
	CString	m_sLogDir;
	CString	m_sExtractDir;
	CString	m_sIgnoreExtensions;
	CString	m_sXBMasterFileDir;
	CString	m_sGCMasterFileDir;
#ifdef _MMI_TARGET_PS2
    CString m_sPS2MasterFileDir;
#endif
	CString	m_sMasterFilename;
	CString	m_sConfigDir;
	CString m_sConfigFilename;
	CString	m_sLocalDir;
	CString m_sLibLockDir;
	// !!Nate
	CString m_sXClientFileDir;
	BOOL m_bAutoRunXClient;

	BOOL m_bXBoxFormat;
	BOOL m_bGCFormat;
	BOOL m_bPCFormat;
#ifdef _MMI_TARGET_PS2
	BOOL m_bPS2Format;	// KJ
#endif
	BOOL m_bOutputDebugPics;
	BOOL m_bShowDifferences;
	BOOL m_bHideNonCompliableFiles;
	BOOL m_bHoldCompileStats;
	BOOL m_bDisableLibrarySupport;
	BOOL m_bUserQuitNetLockWait;
	BOOL m_bMasterFileVersionsChecked;

	CString m_sCurrentMasterFilename;

	CFileInfoArray m_LibFiles;
	CMapStringToPtr m_MapLibFiles;// maps filenames (mike.txt) to pFileInfo pointers in m_LibFiles

	CMasterFileCompile m_MasterFile;

	CFileLock m_FileLock;// used to lock the network file so that PASM can be multi user

	CStringList m_asFilesToDelete;
	CTime m_LastDeleteTime;
	CTime m_DeleteListFileTime;
	
	u32 m_nLastSelectedFilesCount;// used to optimize our status bar updates
		
	void SetupCurrentMasterFilename();
	void SetupLogFile();
	void SaveSettingsOutToDisk( BOOL bSaveConfigFile, BOOL bSaveColumnSortVars );
	void RemoveBogusFiles( CFileInfoArray &rFileInfoArray, BOOL bLogErrors, BOOL bTestExcludedExtensions,
						   BOOL bCheckForLengthyFilenames, BOOL bCheckForZeroByteFiles, BOOL bCheckForDuplicateFiles, 
						   u32 &rnNumLengthyFilenames, u32 &rnNumZeroByteFiles, u32 &rnNumDuplicateFiles );
	void UpdateFileListBox( BOOL bReSortList );
	BOOL AddElementToListBox( CString &rsFilename, CFileInfo *pLib, CFileInfo *pLoc, FDataPrjFile_Entry_t *pEntry, u32 &rnIndex, BOOL bFileIsInIgnoreList=FALSE );	
	typedef enum {
		LOCAL = 0,
		LIBRARY,
		MASTER		
	} Location_e;
	void SelectAll();
	void UnSelectAll();
	void SelectNewAndNewer( Location_e nFrom, Location_e nTo );
	void ReadMasterFileEntries();
	CFileInfo *EntryToFileInfo( const FDataPrjFile_Entry_t *pEntry, CString &rsFilename, BOOL bLocalFile, BOOL bUpdateFilename=TRUE );
	const FDataPrjFile_Entry_t *FileInfoToEntry( CFileInfo *pFileInfo, CString &rsFilename );
	void Compile( BOOL bRebuildAll, BOOL bForceCompilation=FALSE, BOOL bUpdateMasterFileVersionNums=FALSE );
	void RescanAll( BOOL bSetupListBox );
	void UpdateStatusBarFileCounts();
	BOOL CheckMasterFileCompilerVersion();
	BOOL IsExtInIgnoreList( const CString &rsExt );
	void FixIgnoreExtString( CString &rsStr );
	void DeleteSelectedFiles( BOOL bAutoDeleteLocFiles=FALSE, BOOL bAutoDeleteMasEntries=FALSE );
	void CopySelectedLibraryFiles( BOOL bNoPrompts=FALSE );
	void UserQuitWaitingForLibraryLock( BOOL bRescanAll );
	void EnableLibraryButtons( BOOL bEnable );
	void ReadDeleteListFile();
	u32 DeleteLocalFilesListedInTheDeleteListFile( BOOL bUpdateLastDeleteTime, CStringList &rasFilesDeleted );
	u32 DeleteMasterFilesListedInTheDeleteListFile( BOOL bUpdateLastDeleteTime, const CStringList &rasExtraFilesToDelete );
	u32 DeleteAnyTempPasmFiles();
	
	// utils
	BOOL GetFilenameFromFullPath( const CString &sPathname, CString &sFilename );
	void StripTrailingBackSlashFromPathname( CString &rsPathname );
	CFileInfo *GetAidFileInfo( const CFileInfo *pFileInfo, BOOL bLocalFile );
	static BOOL AreTimesEqual( const CTime &rTime1, const CTime &rTime2 );
	CFileInfo *FindCompilableLocalFileFromAidFile( CFileInfo *pAidFileInfo );	
	BOOL IsFilenameALightmap( const CString &rsFilename );
	BOOL IsFilenameALightmap( cchar *pszFilename );
	BOOL GenerateFileTypeUsageFile( cchar *pszExtensionWithPeriod );
	void OptimizeMasterFile( BOOL bLoggedAssetOnly, BOOL bInternationalOptimize );
	BOOL CreateUniqueListOfLogggedFiles( const CString &rsLogFile, CStringList &rasFilenames, BOOL bReplaceUSWithInternationalNames );

public:
	afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeLmapMaxsize();
//	afx_msg void OnBnClickedFilter();
	//afx_msg void OnEnChangeEdit1();
	afx_msg void OnEnChangeSubsample();
	afx_msg void OnToolsGenerateMasterFileList();
	afx_msg void OnToolsCreateNewMasterFileWithOnlyLoggedAssets();
	afx_msg void OnToolsGenerateListOfUnusedAssets();
	afx_msg void OnOptimizeInternationalMasterFile();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PASMDLG_H__9961A1C6_54F9_11D5_AEBF_000102CDD4F3__INCLUDED_)
